# Copyright 2014 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Generate Cobalt bindings (.h and .cpp files).

Based on and borrows heavily from code_generator_v8.py which is used as part
of the bindings generation pipeline in Blink.
"""

import abc
from datetime import date
import os
import sys

# Add blink's binding scripts to the path, so we can import
module_path, module_filename = os.path.split(os.path.realpath(__file__))
blink_script_dir = os.path.normpath(os.path.join(
    module_path, os.pardir, os.pardir, 'third_party', 'blink', 'Source'))
sys.path.append(blink_script_dir)

from bindings.scripts.code_generator_v8 import CodeGeneratorBase  # pylint: disable=g-import-not-at-top
from bindings.scripts.code_generator_v8 import initialize_jinja_env  # pylint: disable=g-import-not-at-top
from bindings.scripts.idl_types import IdlType
from bindings.scripts.idl_types import inherits_interface
import contexts
from name_conversion import convert_to_cobalt_name
from name_conversion import get_interface_name

module_path, module_filename = os.path.split(os.path.realpath(__file__))

# Track the cobalt directory, so we can use it for building relative paths.
cobalt_dir = os.path.normpath(os.path.join(module_path, os.pardir))


def normalize_slashes(path):
  if os.path.sep == '\\':
    return path.replace('\\', '/')
  else:
    return path


def get_implementation_header_path(interface_info):
  """Get the path to the implementation header file.

  The interface file must be under cobalt/...

  Args:
    interface_info: (dict) Contains the information about an interface
      defined in an IDL, as generated by blink's
      compute_interfaces_info_individual.py script
  Returns:
    (str) The path to the corresponding implementation header file, starting
      with the 'cobalt' directory.
  """
  implementation_header_absdir, filename = os.path.split(interface_info[
      'full_path'])

  implementation_header_reldir = os.path.relpath(implementation_header_absdir,
                                                 cobalt_dir)
  assert not os.path.isabs(implementation_header_reldir)
  assert os.pardir not in implementation_header_reldir.split(os.sep)

  root, _ = os.path.splitext(filename)
  header_filename = convert_to_cobalt_name(root) + '.h'
  return normalize_slashes(os.path.join('cobalt', implementation_header_reldir,
                                        header_filename))


def is_global_interface(interface):
  return (('PrimaryGlobal' in interface.extended_attributes) or
          ('Global' in interface.extended_attributes))


def get_indexed_special_operation(interface, special):
  special_operations = list(
      operation for operation in interface.operations
      if (special in operation.specials and operation.arguments and str(
          operation.arguments[0].idl_type) == 'unsigned long'))
  assert len(special_operations) <= 1, (
      'Multiple indexed %ss defined on interface: %s' %
      (special, interface.name))
  return special_operations[0] if len(special_operations) else None


def get_indexed_property_getter(interface):
  getter_operation = get_indexed_special_operation(interface, 'getter')
  assert not getter_operation or len(getter_operation.arguments) == 1
  return getter_operation


def get_indexed_property_setter(interface):
  setter_operation = get_indexed_special_operation(interface, 'setter')
  assert not setter_operation or len(setter_operation.arguments) == 2
  return setter_operation


def get_named_special_operation(interface, special):
  special_operations = list(
      operation for operation in interface.operations
      if (special in operation.specials and operation.arguments and str(
          operation.arguments[0].idl_type) == 'DOMString'))
  assert len(special_operations) <= 1, (
      'Multiple named %ss defined on interface: %s' % (special, interface.name))
  return special_operations[0] if len(special_operations) else None


def get_named_property_getter(interface):
  getter_operation = get_named_special_operation(interface, 'getter')
  assert not getter_operation or len(getter_operation.arguments) == 1
  return getter_operation


def get_named_property_setter(interface):
  setter_operation = get_named_special_operation(interface, 'setter')
  assert not setter_operation or len(setter_operation.arguments) == 2
  return setter_operation


def get_named_property_deleter(interface):
  deleter_operation = get_named_special_operation(interface, 'deleter')
  assert not deleter_operation or len(deleter_operation.arguments) == 1
  return deleter_operation


def get_interface_type_names_from_idl_types(idl_type_list):
  for idl_type in idl_type_list:
    if idl_type:
      if idl_type.is_interface_type:
        yield get_interface_name(idl_type)
      elif idl_type.is_union_type:
        for interface_name in get_interface_type_names_from_idl_types(
            idl_type.member_types):
          yield interface_name


def get_interface_type_names_from_typed_objects(typed_object_list):
  for typed_object in typed_object_list:
    for interface_name in get_interface_type_names_from_idl_types(
        [typed_object.idl_type]):
      yield interface_name

    if hasattr(typed_object, 'arguments'):
      for interface_name in get_interface_type_names_from_typed_objects(
          typed_object.arguments):
        yield interface_name


def split_unsupported_properties(context_list):
  supported = []
  unsupported = []
  for context in context_list:
    if context['unsupported']:
      unsupported.append(context['idl_name'])
    else:
      supported.append(context)
  return supported, unsupported


class CodeGeneratorCobalt(CodeGeneratorBase):
  """Abstract Base code generator class for Cobalt.

  Concrete classes will provide an implementation for generating bindings for a
  specific JavaScript engine implementation.
  """
  __metaclass__ = abc.ABCMeta

  def __init__(self, interfaces_info, templates_dir, cache_dir, output_dir):
    CodeGeneratorBase.__init__(self, interfaces_info, cache_dir, output_dir)
    # CodeGeneratorBase inititalizes this with the v8 template path, so
    # reinitialize it with cobalt's template path

    # Whether the path is absolute or relative affects the cache file name. Use
    # the absolute path to ensure that we use the same path as was used when the
    # cache was prepopulated.
    self.jinja_env = initialize_jinja_env(cache_dir,
                                          os.path.abspath(templates_dir))

  @abc.abstractproperty
  def generated_file_prefix(self):
    """The prefix to prepend to all generated source files."""
    pass

  @abc.abstractproperty
  def expression_generator(self):
    """An instance that implements the ExpressionGenerator class."""
    pass

  def wrapper_class_from_interface_name(self, interface_name):
    return self.generated_file_prefix + interface_name

  def wrapper_header_from_interface_name(self, interface_name):
    attribute_wrapper_class = self.wrapper_class_from_interface_name(
        interface_name)
    return normalize_slashes(attribute_wrapper_class + '.h')

  def output_paths(self, class_name):
    """Construct the filenames for a generated source file.

    Construct the header and cc source file paths based on the
    name of the generated class.

    Args:
      class_name: A string that is the name of the interface as defined
        in the IDL

    Returns:
      (str, str): A tuple of two strings representing the full path of
        the (.h, .cpp) file that will be generated for this interface
    """

    header_path = os.path.join(self.output_dir, class_name + '.h')
    cc_path = os.path.join(self.output_dir, class_name + '.cc')
    return header_path, cc_path

  def render_template(self, header_template_filename, cc_template_filename,
                      template_context):
    header_template = self.jinja_env.get_template(header_template_filename)
    cc_template = self.jinja_env.get_template(cc_template_filename)
    # Make sure extension is .py, not .pyc or .pyo, so doesn't depend on caching
    module_path_pyname = os.path.join(
        module_path, os.path.splitext(module_filename)[0] + '.py')
    # Ensure that posix forward slashes are used
    template_context['code_generator'] = normalize_slashes(os.path.relpath(
        module_path_pyname, cobalt_dir))
    template_context['header_template_path'] = normalize_slashes(
        os.path.relpath(header_template.filename, cobalt_dir))
    template_context['cc_template_path'] = normalize_slashes(os.path.relpath(
        cc_template.filename, cobalt_dir))
    header_text = header_template.render(template_context)
    cc_text = cc_template.render(template_context)
    return header_text, cc_text

  def generate_code_internal(self, definitions, definition_name):
    if definition_name in definitions.interfaces:
      return self.generate_interface_code(
          definitions, definition_name, definitions.interfaces[definition_name])
    if definition_name in definitions.dictionaries:
      return self.generate_dictionary_code(
          definitions, definition_name,
          definitions.dictionaries[definition_name])
    raise ValueError('%s is not in IDL definitions' % definition_name)

  def generate_interface_code(self, definitions, interface_name, interface):
    interface_info = self.interfaces_info[interface_name]
    # Select appropriate Jinja template and contents function
    if interface.is_callback:
      header_template_filename = 'callback-interface.h.template'
      cpp_template_filename = 'callback-interface.cc.template'
    elif interface.is_partial:
      raise NotImplementedError('Partial interfaces not implemented')
    else:
      header_template_filename = 'interface.h.template'
      cpp_template_filename = 'interface.cc.template'

    template_context = self.build_interface_context(interface, interface_info,
                                                    definitions)
    header_text, cc_text = self.render_template(header_template_filename,
                                                cpp_template_filename,
                                                template_context)
    header_path, cc_path = self.output_paths(template_context['binding_class'])
    return ((header_path, header_text), (cc_path, cc_text),)

  def generate_dictionary_code(self, definitions, dictionary_name, dictionary):
    raise NotImplementedError('Dictionaries not implemented')

  def get_interface_components(self, idl_type_name):
    """Get the interface's namespace as a list of namespace components."""
    idl_filename = self.interfaces_info[idl_type_name].get('full_path')
    # Get the IDL filename relative to the cobalt directory, and split the
    # directory to get the list of namespace components.
    idl_filename = os.path.relpath(idl_filename, cobalt_dir)
    components = os.path.dirname(idl_filename).split(os.sep)
    return components

  def referenced_interface_contexts(self, interface_name):
    """Yields information for including and declaring this interface's classes.

    Args:
      interface_name: The name of the interface.
    Yields:
      dict with the following keys:
          fully_qualified_name: Fully qualified name of the class.
          include: Path to the header that defines the class.
          conditional: Symbol on which this interface is conditional compiled.
          is_callback_interface: True iff this is a callback interface.
    """

    conditional = self.interfaces_info[interface_name]['conditional']
    namespace = '::'.join(self.get_interface_components(interface_name))
    full_name = 'cobalt::%s::%s' % (namespace, interface_name)

    # Information about the Cobalt implementation class.
    yield {
        'fully_qualified_name': full_name,
        'include': get_implementation_header_path(self.interfaces_info[
            interface_name]),
        'conditional': conditional,
        'is_callback_interface': interface_name in IdlType.callback_interfaces,
    }
    yield {
        'fully_qualified_name': 'cobalt::%s::%s' % (
            namespace, self.wrapper_class_from_interface_name(interface_name)),
        'include': self.wrapper_header_from_interface_name(interface_name),
        'conditional': conditional,
        'is_callback_interface': interface_name in IdlType.callback_interfaces,
    }

  def build_interface_context(self, interface, interface_info, definitions):
    wrapper_class_name = self.wrapper_class_from_interface_name(interface.name)
    header, _ = self.output_paths(wrapper_class_name)
    generated_header_relative_path = os.path.relpath(header, self.output_dir)
    assert not os.path.isabs(generated_header_relative_path)
    assert os.pardir not in generated_header_relative_path.split(os.sep)

    interface_components = (
        self.get_interface_components(interface.idl_type.name))
    context = {
        # Parameters used for template rendering.
        'today': date.today(),
        'binding_class': wrapper_class_name,
        'fully_qualified_binding_class': (
            '::'.join(interface_components + [wrapper_class_name])),
        'header_file': normalize_slashes(generated_header_relative_path),
        'impl_class': interface.name,
        'fully_qualified_impl_class': (
            '::'.join(interface_components + [interface.name])),
        'interface_name': interface.name,
        'is_event_interface': inherits_interface(interface.name, 'Event'),
        'is_global_interface': is_global_interface(interface),
        'has_interface_object':
            ('NoInterfaceObject' not in interface.extended_attributes),
        'conditional': interface.extended_attributes.get('Conditional', None),
        'add_opaque_roots': interface.extended_attributes.get('AddOpaqueRoots',
                                                              None),
        'get_opaque_root': interface.extended_attributes.get('GetOpaqueRoot',
                                                             None),
    }
    if is_global_interface(interface):
      # Global interface references all interfaces.
      referenced_interface_names = set(
          interface_name
          for interface_name in self.interfaces_info['all_interfaces']
          if not self.interfaces_info[interface_name]['unsupported'] and
          not self.interfaces_info[interface_name]['is_callback_interface'])
      referenced_interface_names.update(IdlType.callback_interfaces)
    else:
      # Build the set of referenced interfaces from this interface's members.
      referenced_interface_names = set()
      referenced_interface_names.update(
          get_interface_type_names_from_typed_objects(interface.attributes))
      referenced_interface_names.update(
          get_interface_type_names_from_typed_objects(interface.operations))
      referenced_interface_names.update(
          get_interface_type_names_from_typed_objects(interface.constructors))
      referenced_interface_names.update(
          get_interface_type_names_from_typed_objects(
              definitions.callback_functions.values()))

    # Build the set of #includes in the header file. Try to keep this small
    # to avoid circular dependency problems.
    header_includes = set()
    if interface.parent:
      header_includes.add(self.wrapper_header_from_interface_name(
          interface.parent))
    header_includes.add(get_implementation_header_path(interface_info))

    attributes = [contexts.attribute_context(interface, attribute, definitions)
                  for attribute in interface.attributes]
    constructor = contexts.get_constructor_context(self.expression_generator,
                                                   interface)
    methods = contexts.get_method_contexts(self.expression_generator, interface)
    constants = [contexts.constant_context(c) for c in interface.constants]

    # Get a list of all the unsupported property names, and remove the
    # unsupported ones from their respective lists
    attributes, unsupported_attribute_names = split_unsupported_properties(
        attributes)
    methods, unsupported_method_names = split_unsupported_properties(methods)
    constants, unsupported_constant_names = split_unsupported_properties(
        constants)

    # Build a set of all interfaces referenced by this interface.
    referenced_interfaces = {}
    for interface_name in referenced_interface_names:
      # Get information for classes related to this interface.
      for referenced_interface_context in self.referenced_interface_contexts(
          interface_name):
        # Store it in the dictionary using fully_qualified_name as the key so
        # we can avoid duplicates
        key = referenced_interface_context['fully_qualified_name']
        referenced_interfaces[key] = referenced_interface_context
    # Now that we have all the referenced interfaces at most once, just get
    # the values(), since the key is duplicated there and we no longer need
    # to lookup by name.
    referenced_interfaces = referenced_interfaces.values()

    all_interfaces = []
    for interface_name in referenced_interface_names:
      if interface_name not in IdlType.callback_interfaces and not (
          self.interfaces_info[interface_name]['unsupported']):
        all_interfaces.append({
            'name': interface_name,
            'conditional': self.interfaces_info[interface_name]['conditional'],
        })

    context['implementation_includes'] = sorted((interface[
        'include'] for interface in referenced_interfaces))
    context['header_includes'] = sorted(header_includes)
    context['attributes'] = [a for a in attributes if not a['is_static']]
    context['static_attributes'] = [a for a in attributes if a['is_static']]
    context['constants'] = constants
    context['constructor'] = constructor
    context['named_constructor'] = interface.extended_attributes.get(
        'NamedConstructor', None)
    context['interface'] = interface
    context['operations'] = [m for m in methods if not m['is_static']]
    context['static_operations'] = [m for m in methods if m['is_static']]
    context['unsupported_interface_properties'] = set(
        unsupported_attribute_names + unsupported_method_names +
        unsupported_constant_names)
    context['unsupported_constructor_properties'] = set(
        unsupported_constant_names)
    if interface.parent:
      context['parent_interface'] = (
          '::'.join(self.get_interface_components(interface.parent)) + '::%s' %
          self.wrapper_class_from_interface_name(interface.parent))
    context['is_exception_interface'] = interface.is_exception
    context['components'] = self.get_interface_components(
        interface.idl_type.name)
    context['forward_declarations'] = sorted(
        referenced_interfaces,
        key=lambda x: x['fully_qualified_name'])
    context['all_interfaces'] = sorted(all_interfaces, key=lambda x: x['name'])
    context['callback_functions'] = definitions.callback_functions.values()
    context['enumerations'] = [
        contexts.enumeration_context(enumeration)
        for enumeration in definitions.enumerations.values()
    ]

    context['stringifier'] = contexts.stringifier_context(interface)
    context['indexed_property_getter'] = contexts.special_method_context(
        interface, get_indexed_property_getter(interface))
    context['indexed_property_setter'] = contexts.special_method_context(
        interface, get_indexed_property_setter(interface))
    context['named_property_getter'] = contexts.special_method_context(
        interface, get_named_property_getter(interface))
    context['named_property_setter'] = contexts.special_method_context(
        interface, get_named_property_setter(interface))
    context['named_property_deleter'] = contexts.special_method_context(
        interface, get_named_property_deleter(interface))

    context['supports_indexed_properties'] = (
        context['indexed_property_getter'] or context[
            'indexed_property_setter'])
    context['supports_named_properties'] = (context['named_property_setter'] or
                                            context['named_property_getter'] or
                                            context['named_property_deleter'])

    return context

################################################################################


def main(argv):
  # If file itself executed, cache templates
  try:
    cache_dir = argv[1]
    templates_dir = argv[2]
    dummy_filename = argv[3]
  except IndexError:
    print 'Usage: %s CACHE_DIR TEMPLATES_DIR DUMMY_FILENAME' % argv[0]
    return 1

  # Cache templates.
  # Whether the path is absolute or relative affects the cache file name. Use
  # the absolute path to ensure that the same path is used when we populate the
  # cache here and when it's read during code generation.
  jinja_env = initialize_jinja_env(cache_dir, os.path.abspath(templates_dir))
  template_filenames = [
      filename
      for filename in os.listdir(templates_dir)
      # Skip .svn, directories, etc.
      if filename.endswith(('.template'))
  ]
  assert template_filenames, 'Expected at least one template to be cached.'
  for template_filename in template_filenames:
    jinja_env.get_template(template_filename)

  # Create a dummy file as output for the build system,
  # since filenames of individual cache files are unpredictable and opaque
  # (they are hashes of the template path, which varies based on environment)
  with open(dummy_filename, 'w') as dummy_file:
    pass  # |open| creates or touches the file


if __name__ == '__main__':
  sys.exit(main(sys.argv))
