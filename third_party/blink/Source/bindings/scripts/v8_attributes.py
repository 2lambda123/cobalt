# Copyright (C) 2013 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# pylint: disable=relative-import

"""Generate template values for attributes.

Extends IdlType with property |constructor_type_name|.

Design doc: http://www.chromium.org/developers/design-documents/idl-compiler
"""

import idl_types
from idl_types import inherits_interface
from v8_globals import includes
import v8_types
import v8_utilities
from v8_utilities import (cpp_name_or_partial, capitalize, cpp_name, has_extended_attribute,
                          has_extended_attribute_value, scoped_name, strip_suffix,
                          uncapitalize, extended_attribute_value_as_list, is_unforgeable,
                          is_legacy_interface_type_checking)


def attribute_context(interface, attribute, interfaces):
    """Creates a Jinja template context for an attribute of an interface.

    Args:
        interface: An interface which |attribute| belongs to
        attribute: An attribute to create the context for
        interfaces: A dict which maps an interface name to the definition
            which can be referred if needed

    Returns:
        A Jinja template context for |attribute|
    """

    idl_type = attribute.idl_type
    base_idl_type = idl_type.base_type
    extended_attributes = attribute.extended_attributes

    idl_type.add_includes_for_type(extended_attributes)
    if idl_type.enum_values:
        includes.add('core/inspector/ConsoleMessage.h')

    # [CheckSecurity]
    is_cross_origin = 'CrossOrigin' in extended_attributes
    is_check_security_for_receiver = (
        has_extended_attribute_value(interface, 'CheckSecurity', 'Receiver') and
        is_cross_origin)
    is_check_security_for_return_value = (
        has_extended_attribute_value(attribute, 'CheckSecurity', 'ReturnValue'))
    if is_check_security_for_receiver or is_check_security_for_return_value:
        includes.add('bindings/core/v8/BindingSecurity.h')
    # [CrossOrigin]
    if has_extended_attribute_value(attribute, 'CrossOrigin', 'Setter'):
        includes.add('bindings/core/v8/V8CrossOriginSetterInfo.h')
    # [Constructor]
    # TODO(yukishiino): Constructors are much like methods although constructors
    # are not methods.  Constructors must be data-type properties, and we can
    # support them as a kind of methods.
    constructor_type = idl_type.constructor_type_name if is_constructor_attribute(attribute) else None
    # [CEReactions]
    is_ce_reactions = 'CEReactions' in extended_attributes
    if is_ce_reactions:
        includes.add('core/dom/custom/CEReactionsScope.h')
    # [CustomElementCallbacks], [Reflect]
    is_custom_element_callbacks = 'CustomElementCallbacks' in extended_attributes
    is_reflect = 'Reflect' in extended_attributes
    if is_custom_element_callbacks or is_reflect:
        includes.add('core/dom/custom/V0CustomElementProcessingStack.h')
    # [PerWorldBindings]
    if 'PerWorldBindings' in extended_attributes:
        assert idl_type.is_wrapper_type or 'LogActivity' in extended_attributes, '[PerWorldBindings] should only be used with wrapper types: %s.%s' % (interface.name, attribute.name)
    # [SaveSameObject]
    is_save_same_object = (
        'SameObject' in attribute.extended_attributes and
        'SaveSameObject' in attribute.extended_attributes)
    if is_save_same_object:
        includes.add('bindings/core/v8/V8PrivateProperty.h')

    if (base_idl_type == 'EventHandler' and
            interface.name in ['Window', 'WorkerGlobalScope'] and
            attribute.name == 'onerror'):
        includes.add('bindings/core/v8/V8ErrorHandler.h')

    cached_attribute_validation_method = extended_attributes.get('CachedAttribute')
    keep_alive_for_gc = is_keep_alive_for_gc(interface, attribute)
    if cached_attribute_validation_method or keep_alive_for_gc:
        includes.add('bindings/core/v8/V8HiddenValue.h')

    # [CachedAccessor]
    is_cached_accessor = 'CachedAccessor' in extended_attributes
    if is_cached_accessor:
        includes.add('bindings/core/v8/V8PrivateProperty.h')

    context = {
        'activity_logging_world_list_for_getter': v8_utilities.activity_logging_world_list(attribute, 'Getter'),  # [ActivityLogging]
        'activity_logging_world_list_for_setter': v8_utilities.activity_logging_world_list(attribute, 'Setter'),  # [ActivityLogging]
        'activity_logging_world_check': v8_utilities.activity_logging_world_check(attribute),  # [ActivityLogging]
        'cached_attribute_validation_method': cached_attribute_validation_method,
        'constructor_type': constructor_type,
        'cpp_name': cpp_name(attribute),
        'cpp_type': idl_type.cpp_type,
        'cpp_type_initializer': idl_type.cpp_type_initializer,
        'deprecate_as': v8_utilities.deprecate_as(attribute),  # [DeprecateAs]
        'enum_type': idl_type.enum_type,
        'enum_values': idl_type.enum_values,
        'exposed_test': v8_utilities.exposed(attribute, interface),  # [Exposed]
        'has_cross_origin_getter':
            has_extended_attribute_value(attribute, 'CrossOrigin', None) or
            has_extended_attribute_value(attribute, 'CrossOrigin', 'Getter'),
        'has_cross_origin_setter': has_extended_attribute_value(attribute, 'CrossOrigin', 'Setter'),
        'has_custom_getter': has_custom_getter(attribute),
        'has_custom_setter': has_custom_setter(attribute),
        'has_setter': has_setter(interface, attribute),
        'idl_type': str(idl_type),  # need trailing [] on array for Dictionary::ConversionContext::setConversionType
        'is_cached_accessor': is_cached_accessor,
        'is_call_with_execution_context': has_extended_attribute_value(attribute, 'CallWith', 'ExecutionContext'),
        'is_call_with_script_state': has_extended_attribute_value(attribute, 'CallWith', 'ScriptState'),
        'is_ce_reactions': is_ce_reactions,
        'is_check_security_for_receiver': is_check_security_for_receiver,
        'is_check_security_for_return_value': is_check_security_for_return_value,
        'is_custom_element_callbacks': is_custom_element_callbacks,
        # TODO(yukishiino): Make all DOM attributes accessor-type properties.
        'is_data_type_property': is_data_type_property(interface, attribute),
        'is_getter_raises_exception':  # [RaisesException]
            'RaisesException' in extended_attributes and
            extended_attributes['RaisesException'] in (None, 'Getter'),
        'is_keep_alive_for_gc': keep_alive_for_gc,
        'is_lenient_this': 'LenientThis' in extended_attributes,
        'is_nullable': idl_type.is_nullable,
        'is_explicit_nullable': idl_type.is_explicit_nullable,
        'is_named_constructor': is_named_constructor_attribute(attribute),
        'is_partial_interface_member':
            'PartialInterfaceImplementedAs' in extended_attributes,
        'is_per_world_bindings': 'PerWorldBindings' in extended_attributes,
        'is_put_forwards': 'PutForwards' in extended_attributes,
        'is_read_only': attribute.is_read_only,
        'is_reflect': is_reflect,
        'is_replaceable': 'Replaceable' in attribute.extended_attributes,
        'is_save_same_object': is_save_same_object,
        'is_static': attribute.is_static,
        'is_url': 'URL' in extended_attributes,
        'is_unforgeable': is_unforgeable(interface, attribute),
        'on_instance': v8_utilities.on_instance(interface, attribute),
        'on_interface': v8_utilities.on_interface(interface, attribute),
        'on_prototype': v8_utilities.on_prototype(interface, attribute),
        'origin_trial_enabled_function': v8_utilities.origin_trial_enabled_function_name(attribute),  # [OriginTrialEnabled]
        'origin_trial_feature_name': v8_utilities.origin_trial_feature_name(attribute),  # [OriginTrialEnabled]
        'use_output_parameter_for_result': idl_type.use_output_parameter_for_result,
        'measure_as': v8_utilities.measure_as(attribute, interface),  # [MeasureAs]
        'name': attribute.name,
        'property_attributes': property_attributes(interface, attribute),
        'reflect_empty': extended_attributes.get('ReflectEmpty'),
        'reflect_invalid': extended_attributes.get('ReflectInvalid', ''),
        'reflect_missing': extended_attributes.get('ReflectMissing'),
        'reflect_only': extended_attribute_value_as_list(attribute, 'ReflectOnly'),
        'runtime_enabled_feature_name': v8_utilities.runtime_enabled_feature_name(attribute),  # [RuntimeEnabled]
        'secure_context_test': v8_utilities.secure_context(attribute, interface),  # [SecureContext]
        'cached_accessor_name': '%s%sCachedAccessor' % (interface.name, attribute.name.capitalize()),
        'world_suffixes': (
            ['', 'ForMainWorld']
            if 'PerWorldBindings' in extended_attributes
            else ['']),  # [PerWorldBindings]
    }

    if is_constructor_attribute(attribute):
        update_constructor_attribute_context(interface, attribute, context)
    if not has_custom_getter(attribute):
        getter_context(interface, attribute, context)
    if not has_custom_setter(attribute) and has_setter(interface, attribute):
        setter_context(interface, attribute, interfaces, context)

    # [CrossOrigin] is incompatible with a number of other attributes, so check
    # for them here.
    if is_cross_origin:
        if context['has_cross_origin_setter'] and context['has_custom_setter']:
            raise Exception('[CrossOrigin] and [Custom] are incompatible on the same setter: %s.%s', interface.name, attribute.name)
        if context['is_per_world_bindings']:
            raise Exception('[CrossOrigin] and [PerWorldBindings] are incompatible: %s.%s', interface.name, attribute.name)
        if context['constructor_type']:
            raise Exception('[CrossOrigin] cannot be used for constructors: %s.%s', interface.name, attribute.name)

    return context


def filter_accessors(attributes):
    return [attribute for attribute in attributes if
            not (attribute['exposed_test'] or
                 attribute['secure_context_test'] or
                 attribute['origin_trial_enabled_function'] or
                 attribute['runtime_enabled_feature_name']) and
            not attribute['is_data_type_property']]


def is_data_attribute(attribute):
    return (not (attribute['exposed_test'] or
                 attribute['secure_context_test'] or
                 attribute['origin_trial_enabled_function'] or
                 attribute['runtime_enabled_feature_name']) and
            attribute['is_data_type_property'])


def is_lazy_data_attribute(attribute):
    return ((attribute['constructor_type'] and not attribute['needs_constructor_getter_callback']) or
            (attribute['idl_type'] == 'Window' and attribute['name'] == 'frames') or
            (attribute['idl_type'] == 'Window' and attribute['name'] == 'self') or
            (attribute['idl_type'] == 'Window' and attribute['name'] == 'window'))


def filter_data_attributes(attributes):
    return [attribute for attribute in attributes if is_data_attribute(attribute) and not is_lazy_data_attribute(attribute)]


def filter_lazy_data_attributes(attributes):
    return [attribute for attribute in attributes if is_data_attribute(attribute) and is_lazy_data_attribute(attribute)]


def filter_origin_trial_enabled(attributes):
    return [attribute for attribute in attributes if
            attribute['origin_trial_feature_name'] and
            not attribute['exposed_test']]


def filter_runtime_enabled(attributes):
    return [attribute for attribute in attributes if
            not (attribute['exposed_test'] or
                 attribute['secure_context_test']) and
            attribute['runtime_enabled_feature_name']]


################################################################################
# Getter
################################################################################

def getter_context(interface, attribute, context):
    idl_type = attribute.idl_type
    base_idl_type = idl_type.base_type
    extended_attributes = attribute.extended_attributes

    cpp_value = getter_expression(interface, attribute, context)
    # Normally we can inline the function call into the return statement to
    # avoid the overhead of using a Ref<> temporary, but for some cases
    # (nullable types, EventHandler, [CachedAttribute], or if there are
    # exceptions), we need to use a local variable.
    # FIXME: check if compilers are smart enough to inline this, and if so,
    # always use a local variable (for readability and CG simplicity).
    if (idl_type.is_explicit_nullable or
            base_idl_type == 'EventHandler' or
            'CachedAttribute' in extended_attributes or
            'ReflectOnly' in extended_attributes or
            context['is_keep_alive_for_gc'] or
            context['is_getter_raises_exception']):
        context['cpp_value_original'] = cpp_value
        cpp_value = 'cppValue'

    def v8_set_return_value_statement(for_main_world=False):
        if context['is_keep_alive_for_gc'] or 'CachedAttribute' in extended_attributes:
            return 'v8SetReturnValue(info, v8Value)'
        return idl_type.v8_set_return_value(
            cpp_value, extended_attributes=extended_attributes, script_wrappable='impl',
            for_main_world=for_main_world, is_static=attribute.is_static)

    context.update({
        'cpp_value': cpp_value,
        'cpp_value_to_v8_value': idl_type.cpp_value_to_v8_value(
            cpp_value=cpp_value, creation_context='holder',
            extended_attributes=extended_attributes),
        'v8_set_return_value_for_main_world': v8_set_return_value_statement(for_main_world=True),
        'v8_set_return_value': v8_set_return_value_statement(),
    })

def getter_expression(interface, attribute, context):
    arguments = []
    this_getter_base_name = getter_base_name(interface, attribute, arguments)
    getter_name = scoped_name(interface, attribute, this_getter_base_name)

    arguments.extend(v8_utilities.call_with_arguments(
        attribute.extended_attributes.get('CallWith')))
    # Members of IDL partial interface definitions are implemented in C++ as
    # static member functions, which for instance members (non-static members)
    # take *impl as their first argument
    if ('PartialInterfaceImplementedAs' in attribute.extended_attributes and
            not attribute.is_static):
        arguments.append('*impl')
    if attribute.idl_type.is_explicit_nullable:
        arguments.append('isNull')
    if context['is_getter_raises_exception']:
        arguments.append('exceptionState')
    if attribute.idl_type.use_output_parameter_for_result:
        arguments.append('result')

    expression = '%s(%s)' % (getter_name, ', '.join(arguments))
    # Needed to handle getter expressions returning Type& as the
    # use site for |expression| expects Type*.
    if attribute.idl_type.is_interface_type and len(arguments) == 0:
        return 'WTF::getPtr(%s)' % expression
    return expression


CONTENT_ATTRIBUTE_GETTER_NAMES = {
    'boolean': 'fastHasAttribute',
    'long': 'getIntegralAttribute',
    'unsigned long': 'getUnsignedIntegralAttribute',
}


def getter_base_name(interface, attribute, arguments):
    extended_attributes = attribute.extended_attributes

    if 'Reflect' not in extended_attributes:
        return uncapitalize(cpp_name(attribute))

    content_attribute_name = extended_attributes['Reflect'] or attribute.name.lower()
    if content_attribute_name in ['class', 'id', 'name']:
        # Special-case for performance optimization.
        return 'get%sAttribute' % content_attribute_name.capitalize()

    arguments.append(scoped_content_attribute_name(interface, attribute))

    base_idl_type = attribute.idl_type.base_type
    if base_idl_type in CONTENT_ATTRIBUTE_GETTER_NAMES:
        return CONTENT_ATTRIBUTE_GETTER_NAMES[base_idl_type]
    if 'URL' in attribute.extended_attributes:
        return 'getURLAttribute'
    return 'fastGetAttribute'


def is_keep_alive_for_gc(interface, attribute):
    idl_type = attribute.idl_type
    base_idl_type = idl_type.base_type
    extended_attributes = attribute.extended_attributes
    return (
        # For readonly attributes, for performance reasons we keep the attribute
        # wrapper alive while the owner wrapper is alive, because the attribute
        # never changes.
        (attribute.is_read_only and
         idl_type.is_wrapper_type and
         # There are some exceptions, however:
         not(
             # Node lifetime is managed by object grouping.
             inherits_interface(interface.name, 'Node') or
             inherits_interface(base_idl_type, 'Node') or
             # A self-reference is unnecessary.
             attribute.name == 'self' or
             # FIXME: Remove these hard-coded hacks.
             base_idl_type in ['EventTarget', 'Window'] or
             base_idl_type.startswith(('HTML', 'SVG')))))


################################################################################
# Setter
################################################################################

def setter_context(interface, attribute, interfaces, context):
    if 'PutForwards' in attribute.extended_attributes:
        # Use target interface and attribute in place of original interface and
        # attribute from this point onwards.
        target_interface_name = attribute.idl_type.base_type
        target_attribute_name = attribute.extended_attributes['PutForwards']
        interface = interfaces[target_interface_name]
        try:
            attribute = next(candidate
                             for candidate in interface.attributes
                             if candidate.name == target_attribute_name)
        except StopIteration:
            raise Exception('[PutForward] target not found:\n'
                            'Attribute "%s" is not present in interface "%s"' %
                            (target_attribute_name, target_interface_name))

    if ('Replaceable' in attribute.extended_attributes):
        context['cpp_setter'] = 'v8CallBoolean(info.Holder()->CreateDataProperty(info.GetIsolate()->GetCurrentContext(), propertyName, v8Value))'
        return

    extended_attributes = attribute.extended_attributes
    idl_type = attribute.idl_type

    # [RaisesException], [RaisesException=Setter]
    is_setter_raises_exception = (
        'RaisesException' in extended_attributes and
        extended_attributes['RaisesException'] in [None, 'Setter'])
    # [LegacyInterfaceTypeChecking]
    has_type_checking_interface = (
        not is_legacy_interface_type_checking(interface, attribute) and
        idl_type.is_wrapper_type)

    context.update({
        'has_setter_exception_state':
            is_setter_raises_exception or has_type_checking_interface or
            idl_type.v8_conversion_needs_exception_state,
        'has_type_checking_interface': has_type_checking_interface,
        'is_setter_call_with_execution_context': has_extended_attribute_value(
            attribute, 'SetterCallWith', 'ExecutionContext'),
        'is_setter_raises_exception': is_setter_raises_exception,
        'v8_value_to_local_cpp_value': idl_type.v8_value_to_local_cpp_value(
            extended_attributes, 'v8Value', 'cppValue'),
    })

    # setter_expression() depends on context values we set above.
    context['cpp_setter'] = setter_expression(interface, attribute, context)


def setter_expression(interface, attribute, context):
    extended_attributes = attribute.extended_attributes
    arguments = v8_utilities.call_with_arguments(
        extended_attributes.get('SetterCallWith') or
        extended_attributes.get('CallWith'))

    this_setter_base_name = setter_base_name(interface, attribute, arguments)
    setter_name = scoped_name(interface, attribute, this_setter_base_name)

    # Members of IDL partial interface definitions are implemented in C++ as
    # static member functions, which for instance members (non-static members)
    # take *impl as their first argument
    if ('PartialInterfaceImplementedAs' in extended_attributes and
            not attribute.is_static):
        arguments.append('*impl')
    idl_type = attribute.idl_type
    if idl_type.base_type == 'EventHandler':
        getter_name = scoped_name(interface, attribute, cpp_name(attribute))
        context['event_handler_getter_expression'] = '%s(%s)' % (
            getter_name, ', '.join(arguments))
        if (interface.name in ['Window', 'WorkerGlobalScope'] and
                attribute.name == 'onerror'):
            includes.add('bindings/core/v8/V8ErrorHandler.h')
            arguments.append(
                'V8EventListenerHelper::ensureEventListener<V8ErrorHandler>(' +
                'v8Value, true, ScriptState::forReceiverObject(info))')
        else:
            arguments.append(
                'V8EventListenerHelper::getEventListener(' +
                'ScriptState::forReceiverObject(info), v8Value, true, ' +
                'ListenerFindOrCreate)')
    else:
        arguments.append('cppValue')
    if context['is_setter_raises_exception']:
        arguments.append('exceptionState')

    return '%s(%s)' % (setter_name, ', '.join(arguments))


CONTENT_ATTRIBUTE_SETTER_NAMES = {
    'boolean': 'setBooleanAttribute',
    'long': 'setIntegralAttribute',
    'unsigned long': 'setUnsignedIntegralAttribute',
}


def setter_base_name(interface, attribute, arguments):
    if 'Reflect' not in attribute.extended_attributes:
        return 'set%s' % capitalize(cpp_name(attribute))
    arguments.append(scoped_content_attribute_name(interface, attribute))

    base_idl_type = attribute.idl_type.base_type
    if base_idl_type in CONTENT_ATTRIBUTE_SETTER_NAMES:
        return CONTENT_ATTRIBUTE_SETTER_NAMES[base_idl_type]
    return 'setAttribute'


def scoped_content_attribute_name(interface, attribute):
    content_attribute_name = attribute.extended_attributes['Reflect'] or attribute.name.lower()
    if interface.name.startswith('SVG'):
        namespace = 'SVGNames'
    else:
        namespace = 'HTMLNames'
    includes.add('core/%s.h' % namespace)
    return '%s::%sAttr' % (namespace, content_attribute_name)


################################################################################
# Attribute configuration
################################################################################

# Property descriptor's {writable: boolean}
def is_writable(attribute):
    return (not attribute.is_read_only or
            'PutForwards' in attribute.extended_attributes or
            'Replaceable' in attribute.extended_attributes)


def is_data_type_property(interface, attribute):
    if 'CachedAccessor' in attribute.extended_attributes:
        return False
    return (is_constructor_attribute(attribute) or
            'CrossOrigin' in attribute.extended_attributes)


# [PutForwards], [Replaceable]
def has_setter(interface, attribute):
    if (is_data_type_property(interface, attribute) and
        (is_constructor_attribute(attribute) or
         'Replaceable' in attribute.extended_attributes)):
        return False

    return is_writable(attribute)


# [NotEnumerable], [Unforgeable]
def property_attributes(interface, attribute):
    extended_attributes = attribute.extended_attributes
    property_attributes_list = []
    if ('NotEnumerable' in extended_attributes or
            is_constructor_attribute(attribute)):
        property_attributes_list.append('v8::DontEnum')
    if is_unforgeable(interface, attribute):
        property_attributes_list.append('v8::DontDelete')
    if not is_writable(attribute):
        property_attributes_list.append('v8::ReadOnly')
    return property_attributes_list or ['v8::None']


# [Custom], [Custom=Getter]
def has_custom_getter(attribute):
    extended_attributes = attribute.extended_attributes
    return ('Custom' in extended_attributes and
            extended_attributes['Custom'] in [None, 'Getter'])


# [Custom], [Custom=Setter]
def has_custom_setter(attribute):
    extended_attributes = attribute.extended_attributes
    return (not attribute.is_read_only and
            'Custom' in extended_attributes and
            extended_attributes['Custom'] in [None, 'Setter'])


################################################################################
# Constructors
################################################################################

idl_types.IdlType.constructor_type_name = property(
    lambda self: strip_suffix(self.base_type, 'Constructor'))


def is_constructor_attribute(attribute):
    return attribute.idl_type.name.endswith('Constructor')


def is_named_constructor_attribute(attribute):
    return attribute.idl_type.name.endswith('ConstructorConstructor')


def update_constructor_attribute_context(interface, attribute, context):
    context['needs_constructor_getter_callback'] = context['measure_as'] or context['deprecate_as']
