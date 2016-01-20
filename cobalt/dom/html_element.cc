/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cobalt/dom/html_element.h"

#include <map>

#include "base/string_number_conversions.h"
#include "cobalt/cssom/absolute_url_value.h"
#include "cobalt/cssom/cascaded_style.h"
#include "cobalt/cssom/computed_style.h"
#include "cobalt/cssom/css_parser.h"
#include "cobalt/cssom/css_style_sheet.h"
#include "cobalt/cssom/keyword_value.h"
#include "cobalt/cssom/property_list_value.h"
#include "cobalt/cssom/selector_tree.h"
#include "cobalt/dom/csp_delegate.h"
#include "cobalt/dom/document.h"
#include "cobalt/dom/dom_animatable.h"
#include "cobalt/dom/dom_string_map.h"
#include "cobalt/dom/focus_event.h"
#include "cobalt/dom/html_anchor_element.h"
#include "cobalt/dom/html_body_element.h"
#include "cobalt/dom/html_br_element.h"
#include "cobalt/dom/html_div_element.h"
#include "cobalt/dom/html_element_context.h"
#include "cobalt/dom/html_element_factory.h"
#include "cobalt/dom/html_head_element.h"
#include "cobalt/dom/html_heading_element.h"
#include "cobalt/dom/html_html_element.h"
#include "cobalt/dom/html_image_element.h"
#include "cobalt/dom/html_link_element.h"
#include "cobalt/dom/html_meta_element.h"
#include "cobalt/dom/html_paragraph_element.h"
#include "cobalt/dom/html_script_element.h"
#include "cobalt/dom/html_span_element.h"
#include "cobalt/dom/html_style_element.h"
#include "cobalt/dom/html_title_element.h"
#include "cobalt/dom/html_unknown_element.h"
#include "cobalt/dom/html_video_element.h"

namespace cobalt {
namespace dom {

scoped_refptr<DOMStringMap> HTMLElement::dataset() {
  scoped_refptr<DOMStringMap> dataset(dataset_);
  if (!dataset) {
    // Create a new instance and store a weak reference.
    dataset = new DOMStringMap(this);
    dataset_ = dataset->AsWeakPtr();
  }
  return dataset;
}

int32 HTMLElement::tab_index() const {
  int32 tabindex;
  bool success =
      base::StringToInt32(GetAttribute("tabindex").value_or(""), &tabindex);
  if (!success) {
    LOG(WARNING) << "Element's tabindex is not an integer.";
  }
  return tabindex;
}

void HTMLElement::set_tab_index(int32 tab_index) {
  SetAttribute("tabindex", base::Int32ToString(tab_index));
}

void HTMLElement::Focus() {
  if (!IsFocusable()) {
    return;
  }

  Document* document = node_document();
  Element* old_active_element = document->active_element();
  if (old_active_element == this->AsElement()) {
    return;
  }

  document->SetActiveElement(this);

  if (old_active_element) {
    old_active_element->DispatchEvent(new FocusEvent("blur", this));
  }

  DispatchEvent(new FocusEvent("focus", old_active_element));
}

void HTMLElement::Blur() {
  Document* document = node_document();
  if (document->active_element() == this->AsElement()) {
    document->SetActiveElement(NULL);

    DispatchEvent(new FocusEvent("blur", NULL));
  }
}

// Algorithm for GetClientRects:
//   https://www.w3.org/TR/2013/WD-cssom-view-20131217/#dom-element-getclientrects
scoped_refptr<DOMRectList> HTMLElement::GetClientRects() {
  DCHECK(node_document());
  node_document()->DoSynchronousLayout();

  // 1. If the element on which it was invoked does not have an associated
  // layout box return an empty DOMRectList object and stop this algorithm.
  if (!layout_boxes_) {
    return make_scoped_refptr(new DOMRectList());
  }

  // The remaining steps are implemented in LayoutBoxes::GetClientRects().
  return layout_boxes_->GetClientRects();
}

// Algorithm for client_top:
//   https://www.w3.org/TR/2013/WD-cssom-view-20131217/#dom-element-clienttop
float HTMLElement::client_top() {
  DCHECK(node_document());
  node_document()->DoSynchronousLayout();

  // 1. If the element has no associated CSS layout box or if the CSS layout box
  // is inline, return zero.
  if (!layout_boxes_ || layout_boxes_->IsInlineLevel()) {
    return 0.0f;
  }
  // 2. Return the computed value of the 'border-top-width' property plus the
  // height of any scrollbar rendered between the top padding edge and the top
  // border edge, ignoring any transforms that apply to the element and its
  // ancestors.
  return layout_boxes_->GetBorderTopWidth();
}

// Algorithm for client_left:
//   https://www.w3.org/TR/2013/WD-cssom-view-20131217/#dom-element-clientleft
float HTMLElement::client_left() {
  DCHECK(node_document());
  node_document()->DoSynchronousLayout();

  // 1. If the element has no associated CSS layout box or if the CSS layout box
  // is inline, return zero.
  if (!layout_boxes_ || layout_boxes_->IsInlineLevel()) {
    return 0.0f;
  }
  // 2. Return the computed value of the 'border-left-width' property plus the
  // width of any scrollbar rendered between the left padding edge and the left
  // border edge, ignoring any transforms that apply to the element and its
  // ancestors.
  return layout_boxes_->GetBorderLeftWidth();
}

// Algorithm for client_width:
//   https://www.w3.org/TR/2013/WD-cssom-view-20131217/#dom-element-clientwidth
float HTMLElement::client_width() {
  DCHECK(node_document());
  node_document()->DoSynchronousLayout();

  // 1. If the element has no associated CSS layout box or if the CSS layout box
  // is inline, return zero.
  if (!layout_boxes_ || layout_boxes_->IsInlineLevel()) {
    return 0.0f;
  }

  // 2. If the element is the root element, return the viewport width.
  if (IsRootElement()) {
    return layout_boxes_->GetMarginEdgeWidth();
  }

  // 3. Return the width of the padding edge, ignoring any transforms that apply
  // to the element and its ancestors.
  return layout_boxes_->GetPaddingEdgeWidth();
}

// Algorithm for client_height:
//   https://www.w3.org/TR/2013/WD-cssom-view-20131217/#dom-element-clientheight
float HTMLElement::client_height() {
  DCHECK(node_document());
  node_document()->DoSynchronousLayout();

  // 1. If the element has no associated CSS layout box or if the CSS layout box
  // is inline, return zero.
  if (!layout_boxes_ || layout_boxes_->IsInlineLevel()) {
    return 0.0f;
  }

  // 2. If the element is the root element, return the viewport height.
  if (IsRootElement()) {
    return layout_boxes_->GetMarginEdgeHeight();
  }

  // Return the height of the padding edge, ignoring any transforms that apply
  // to the element and its ancestors.
  return layout_boxes_->GetPaddingEdgeHeight();
}

// Algorithm for offsetParent:
//   https://www.w3.org/TR/2013/WD-cssom-view-20131217/#dom-htmlelement-offsetparent
scoped_refptr<Element> HTMLElement::offset_parent() {
  DCHECK(node_document());
  node_document()->DoSynchronousLayout();

  // 1. If any of the following holds true return null and terminate this
  //    algorithm:
  //    . The element does not have an associated CSS layout box.
  //    . The element is the root element.
  //    . The element is the HTML body element.
  //    . The element's computed value of the 'position' property is 'fixed'.
  DCHECK(computed_style());
  if (!layout_boxes_ || IsRootElement() || AsHTMLBodyElement() ||
      computed_style()->position() == cssom::KeywordValue::GetFixed()) {
    return scoped_refptr<Element>();
  }

  // 2. Return the nearest ancestor element of the element for which at least
  //    one of the following is true and terminate this algorithm if such an
  //    ancestor is found:
  //    . The computed value of the 'position' property is not 'static'.
  //    . It is the HTML body element.
  for (scoped_refptr<Node> ancestor_node = parent_node(); ancestor_node;
       ancestor_node = ancestor_node->parent_node()) {
    scoped_refptr<Element> ancestor_element = ancestor_node->AsElement();
    if (!ancestor_element) {
      continue;
    }
    scoped_refptr<HTMLElement> ancestor_html_element =
        ancestor_element->AsHTMLElement();
    if (!ancestor_html_element) {
      continue;
    }
    DCHECK(ancestor_html_element->computed_style());
    if (ancestor_html_element->AsHTMLBodyElement() ||
        ancestor_html_element->computed_style()->position() !=
            cssom::KeywordValue::GetStatic()) {
      return ancestor_element;
    }
  }

  // 3. Return null.
  return scoped_refptr<Element>();
}

// Algorithm for offset_top:
//   https://www.w3.org/TR/2013/WD-cssom-view-20131217/#dom-htmlelement-offsettop
float HTMLElement::offset_top() {
  DCHECK(node_document());
  node_document()->DoSynchronousLayout();

  // 1. If the element is the HTML body element or does not have any associated
  // CSS layout box return zero and terminate this algorithm.
  if (!layout_boxes_ || AsHTMLBodyElement()) {
    return 0.0f;
  }

  // 2. If the offsetParent of the element is null return the y-coordinate of
  // the top border edge of the first CSS layout box associated with the
  // element, relative to the initial containing block origin, ignoring any
  // transforms that apply to the element and its ancestors, and terminate this
  // algorithm.
  scoped_refptr<Element> offset_parent_element = offset_parent();
  if (!offset_parent_element) {
    return layout_boxes_->GetBorderEdgeTop();
  }

  // 3. Return the result of subtracting the y-coordinate of the top padding
  // edge of the first CSS layout box associated with the offsetParent of the
  // element from the y-coordinate of the top border edge of the first CSS
  // layout box associated with the element, relative to the initial containing
  // block origin, ignoring any transforms that apply to the element and its
  // ancestors.
  scoped_refptr<HTMLElement> offset_parent_html_element =
      offset_parent_element->AsHTMLElement();
  DCHECK(offset_parent_html_element);
  DCHECK(offset_parent_html_element->layout_boxes());
  return layout_boxes_->GetBorderEdgeTop() -
         offset_parent_html_element->layout_boxes()->GetPaddingEdgeTop();
}

// Algorithm for offset_left:
//   https://www.w3.org/TR/2013/WD-cssom-view-20131217/#dom-htmlelement-offsetleft
float HTMLElement::offset_left() {
  DCHECK(node_document());
  node_document()->DoSynchronousLayout();

  // 1. If the element is the HTML body element or does not have any associated
  // CSS layout box return zero and terminate this algorithm.
  if (!layout_boxes_ || AsHTMLBodyElement()) {
    return 0.0f;
  }

  // 2. If the offsetParent of the element is null return the x-coordinate of
  // the left border edge of the first CSS layout box associated with the
  // element, relative to the initial containing block origin, ignoring any
  // transforms that apply to the element and its ancestors, and terminate this
  // algorithm.
  scoped_refptr<Element> offset_parent_element = offset_parent();
  if (!offset_parent_element) {
    return layout_boxes_->GetBorderEdgeLeft();
  }

  // 3. Return the result of subtracting the x-coordinate of the left padding
  // edge of the first CSS layout box associated with the offsetParent of the
  // element from the x-coordinate of the left border edge of the first CSS
  // layout box associated with the element, relative to the initial containing
  // block origin, ignoring any transforms that apply to the element and its
  // ancestors.
  scoped_refptr<HTMLElement> offset_parent_html_element =
      offset_parent_element->AsHTMLElement();
  DCHECK(offset_parent_html_element);
  DCHECK(offset_parent_html_element->layout_boxes());
  return layout_boxes_->GetBorderEdgeLeft() -
         offset_parent_html_element->layout_boxes()->GetPaddingEdgeLeft();
}

// Algorithm for offset_width:
//   https://www.w3.org/TR/2013/WD-cssom-view-20131217/#dom-htmlelement-offsetwidth
float HTMLElement::offset_width() {
  DCHECK(node_document());
  node_document()->DoSynchronousLayout();

  // 1. If the element does not have any associated CSS layout box return zero
  // and terminate this algorithm.
  if (!layout_boxes_) {
    return 0.0f;
  }

  // 2. Return the border edge width of the first CSS layout box associated with
  // the element, ignoring any transforms that apply to the element and its
  // ancestors.
  return layout_boxes_->GetBorderEdgeWidth();
}

// Algorithm for offset_height:
//   https://www.w3.org/TR/2013/WD-cssom-view-20131217/#dom-htmlelement-offsetheight
float HTMLElement::offset_height() {
  DCHECK(node_document());
  node_document()->DoSynchronousLayout();

  // 1. If the element does not have any associated CSS layout box return zero
  // and terminate this algorithm.
  if (!layout_boxes_) {
    return 0.0f;
  }

  // 2. Return the border edge height of the first CSS layout box associated
  // with the element, ignoring any transforms that apply to the element and its
  // ancestors.
  return layout_boxes_->GetBorderEdgeHeight();
}

scoped_refptr<Node> HTMLElement::Duplicate() const {
  Document* document = node_document();
  DCHECK(document->html_element_context()->html_element_factory());
  scoped_refptr<HTMLElement> new_html_element =
      document->html_element_context()
          ->html_element_factory()
          ->CreateHTMLElement(document, tag_name());
  new_html_element->CopyAttributes(*this);

  return new_html_element;
}

base::optional<std::string> HTMLElement::GetStyleAttribute() const {
  base::optional<std::string> value = Element::GetStyleAttribute();
  return value.value_or(style_->css_text());
}

void HTMLElement::SetStyleAttribute(const std::string& value) {
  style_->set_css_text(value);
  Element::SetStyleAttribute(value);
}

void HTMLElement::RemoveStyleAttribute() {
  style_->set_css_text("");
  Element::RemoveStyleAttribute();
}

void HTMLElement::OnCSSMutation() {
  // Invalidate the computed style of this node.
  computed_style_valid_ = false;

  // Remove the style attribute value from the Element.
  Element::RemoveStyleAttribute();

  node_document()->OnElementInlineStyleMutation();
}

scoped_refptr<HTMLAnchorElement> HTMLElement::AsHTMLAnchorElement() {
  return NULL;
}

scoped_refptr<HTMLBodyElement> HTMLElement::AsHTMLBodyElement() { return NULL; }

scoped_refptr<HTMLBRElement> HTMLElement::AsHTMLBRElement() { return NULL; }

scoped_refptr<HTMLDivElement> HTMLElement::AsHTMLDivElement() { return NULL; }

scoped_refptr<HTMLHeadElement> HTMLElement::AsHTMLHeadElement() { return NULL; }

scoped_refptr<HTMLHeadingElement> HTMLElement::AsHTMLHeadingElement() {
  return NULL;
}

scoped_refptr<HTMLHtmlElement> HTMLElement::AsHTMLHtmlElement() { return NULL; }

scoped_refptr<HTMLImageElement> HTMLElement::AsHTMLImageElement() {
  return NULL;
}

scoped_refptr<HTMLLinkElement> HTMLElement::AsHTMLLinkElement() { return NULL; }

scoped_refptr<HTMLMetaElement> HTMLElement::AsHTMLMetaElement() { return NULL; }

scoped_refptr<HTMLParagraphElement> HTMLElement::AsHTMLParagraphElement() {
  return NULL;
}

scoped_refptr<HTMLScriptElement> HTMLElement::AsHTMLScriptElement() {
  return NULL;
}

scoped_refptr<HTMLSpanElement> HTMLElement::AsHTMLSpanElement() { return NULL; }

scoped_refptr<HTMLStyleElement> HTMLElement::AsHTMLStyleElement() {
  return NULL;
}

scoped_refptr<HTMLTitleElement> HTMLElement::AsHTMLTitleElement() {
  return NULL;
}

scoped_refptr<HTMLUnknownElement> HTMLElement::AsHTMLUnknownElement() {
  return NULL;
}

scoped_refptr<HTMLVideoElement> HTMLElement::AsHTMLVideoElement() {
  return NULL;
}

void HTMLElement::InvalidateMatchingRules() {
  if (!matching_rules_valid_) {
    return;
  }

  matching_rules_valid_ = false;
  computed_style_valid_ = false;

  matching_rules_.clear();
  rule_matching_state_.matching_nodes.clear();
  rule_matching_state_.descendant_potential_nodes.clear();
  rule_matching_state_.following_sibling_potential_nodes.clear();
  for (int pseudo_element_type = 0; pseudo_element_type < kMaxPseudoElementType;
       ++pseudo_element_type) {
    if (pseudo_elements_[pseudo_element_type]) {
      pseudo_elements_[pseudo_element_type]->ClearMatchingRules();
    }
  }

  // Invalidate matching rules on all children.
  for (Element* element = first_element_child(); element;
       element = element->next_element_sibling()) {
    HTMLElement* html_element = element->AsHTMLElement();
    DCHECK(html_element);
    html_element->InvalidateMatchingRules();
  }

  // Invalidate matching rules on all following siblings if sibling combinators
  // are used.
  if (node_document()->selector_tree()->has_sibling_combinators()) {
    for (Element* element = next_element_sibling(); element;
         element = element->next_element_sibling()) {
      HTMLElement* html_element = element->AsHTMLElement();
      DCHECK(html_element);
      html_element->InvalidateMatchingRules();
    }
  }
}

namespace {

scoped_refptr<cssom::CSSStyleDeclarationData>
PromoteMatchingRulesToComputedStyle(
    cssom::RulesWithCascadePriority* matching_rules,
    cssom::GURLMap* property_key_to_base_url_map,
    const scoped_refptr<const cssom::CSSStyleDeclarationData>& inline_style,
    const scoped_refptr<const cssom::CSSStyleDeclarationData>&
        parent_computed_style,
    const base::TimeDelta& style_change_event_time,
    cssom::TransitionSet* css_transitions,
    const scoped_refptr<const cssom::CSSStyleDeclarationData>&
        previous_computed_style,
    cssom::AnimationSet* css_animations,
    const cssom::CSSKeyframesRule::NameMap& keyframes_map) {
  scoped_refptr<cssom::CSSStyleDeclarationData> computed_style =
      new cssom::CSSStyleDeclarationData();

  // Get the element's inline styles.
  computed_style->AssignFrom(*inline_style);

  // Select the winning value for each property by performing the cascade,
  // that is, apply values from matching rules on top of inline style, taking
  // into account rule specificity and location in the source file, as well as
  // property declaration importance.
  cssom::PromoteToCascadedStyle(computed_style, matching_rules,
                                property_key_to_base_url_map);

  // Lastly, absolutize the values, if possible. Start by resolving "initial"
  // and "inherit" keywords (which gives us what the specification refers to
  // as "specified style").  Then, convert length units and percentages into
  // pixels, convert color keywords into RGB triplets, and so on.  For certain
  // properties, like "font-family", computed value is the same as specified
  // value. Declarations that cannot be absolutized easily, like "width: auto;",
  // will be resolved during layout.
  cssom::PromoteToComputedStyle(computed_style, parent_computed_style,
                                property_key_to_base_url_map);

  if (previous_computed_style) {
    // Now that we have updated our computed style, compare it to the previous
    // style and see if we need to adjust our animations.
    css_transitions->UpdateTransitions(
        style_change_event_time, *previous_computed_style, *computed_style);
  }
  // Update the set of currently running animations.
  css_animations->Update(style_change_event_time, *computed_style,
                         keyframes_map);

  return computed_style;
}

bool NewComputedStyleInvalidatesLayoutBoxes(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>&
        old_computed_style,
    const scoped_refptr<cssom::CSSStyleDeclarationData>& new_computed_style) {
  // FIXE(***REMOVED***): Only invalidate layout boxes when a property that is used for
  // box generation is modified. We currently have to also invalidate when any
  // inheritable property is modified, because AnonymousBlockBox and TextBox use
  // GetComputedStyleOfAnonymousBox() to store a copy of them that won't
  // automatically get updated when the style() in a ComputedStyleState gets
  // updated.
  return !old_computed_style->display()->Equals(
             *new_computed_style->display()) ||
         !old_computed_style->content()->Equals(
             *new_computed_style->content()) ||
         !old_computed_style->color()->Equals(*new_computed_style->color()) ||
         !old_computed_style->font_family()->Equals(
             *new_computed_style->font_family()) ||
         !old_computed_style->font_size()->Equals(
             *new_computed_style->font_size()) ||
         !old_computed_style->font_style()->Equals(
             *new_computed_style->font_style()) ||
         !old_computed_style->font_weight()->Equals(
             *new_computed_style->font_weight()) ||
         !old_computed_style->line_height()->Equals(
             *new_computed_style->line_height()) ||
         !old_computed_style->overflow_wrap()->Equals(
             *new_computed_style->overflow_wrap()) ||
         !old_computed_style->text_align()->Equals(
             *new_computed_style->text_align()) ||
         !old_computed_style->text_indent()->Equals(
             *new_computed_style->text_indent()) ||
         !old_computed_style->text_transform()->Equals(
             *new_computed_style->text_transform()) ||
         !old_computed_style->visibility()->Equals(
             *new_computed_style->visibility()) ||
         !old_computed_style->white_space()->Equals(
             *new_computed_style->white_space());
}

}  // namespace

void HTMLElement::UpdateComputedStyle(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>&
        parent_computed_style,
    const base::TimeDelta& style_change_event_time) {
  Document* document = node_document();
  DCHECK(document) << "Element should be attached to document in order to "
                      "participate in layout.";

  // TODO(***REMOVED***): It maybe helpful to generalize this mapping framework in the
  // future to allow more data and context about where a cssom::PropertyValue
  // came from.
  cssom::GURLMap property_key_to_base_url_map;
  property_key_to_base_url_map[cssom::kBackgroundImageProperty] =
      document->url_as_gurl();

  scoped_refptr<cssom::CSSStyleDeclarationData> new_computed_style =
      PromoteMatchingRulesToComputedStyle(
          matching_rules(), &property_key_to_base_url_map, style_->data(),
          parent_computed_style, style_change_event_time, &css_transitions_,
          computed_style(), &css_animations_, document->keyframes_map());

  // If there is no previous computed style, there should also be no layout
  // boxes, and nothing has to be invalidated.
  bool invalidate_layout_boxes = false;
  DCHECK(computed_style() || NULL == layout_boxes());
  if (computed_style() && NewComputedStyleInvalidatesLayoutBoxes(
                              computed_style(), new_computed_style)) {
    invalidate_layout_boxes = true;
  }
  set_computed_style(new_computed_style);

  // Update cached background images after resolving the urls in
  // background_image CSS property of the computed style, so we have all the
  // information to get the cached background images.
  UpdateCachedBackgroundImagesFromComputedStyle();

  // Promote the matching rules for all known pseudo elements.
  for (int pseudo_element_type = 0; pseudo_element_type < kMaxPseudoElementType;
       ++pseudo_element_type) {
    if (pseudo_elements_[pseudo_element_type]) {
      scoped_refptr<cssom::CSSStyleDeclarationData>
          pseudo_element_computed_style = PromoteMatchingRulesToComputedStyle(
              pseudo_elements_[pseudo_element_type]->matching_rules(),
              &property_key_to_base_url_map, style_->data(), computed_style(),
              style_change_event_time,
              pseudo_elements_[pseudo_element_type]->css_transitions(),
              pseudo_elements_[pseudo_element_type]->computed_style(),
              pseudo_elements_[pseudo_element_type]->css_animations(),
              document->keyframes_map());

      if (!invalidate_layout_boxes &&
          pseudo_elements_[pseudo_element_type]->computed_style() &&
          NewComputedStyleInvalidatesLayoutBoxes(
              pseudo_elements_[pseudo_element_type]->computed_style(),
              pseudo_element_computed_style)) {
        invalidate_layout_boxes = true;
      }
      pseudo_elements_[pseudo_element_type]->set_computed_style(
          pseudo_element_computed_style);
    }
  }

  if (invalidate_layout_boxes) {
    InvalidateLayoutBoxesFromNodeAndAncestors();
    InvalidateLayoutBoxesFromNodeAndDescendants();
  }

  computed_style_valid_ = true;
}

void HTMLElement::UpdateComputedStyleRecursively(
    const scoped_refptr<const cssom::CSSStyleDeclarationData>&
        parent_computed_style,
    const base::TimeDelta& style_change_event_time, bool ancestors_were_valid) {
  bool is_valid = ancestors_were_valid && computed_style_valid_;
  if (!is_valid) {
    UpdateComputedStyle(parent_computed_style, style_change_event_time);
    DCHECK(computed_style_valid_);
  }

  // Update computed style for this element's descendants.
  for (Element* element = first_element_child(); element;
       element = element->next_element_sibling()) {
    HTMLElement* html_element = element->AsHTMLElement();
    DCHECK(html_element);
    html_element->UpdateComputedStyleRecursively(
        computed_style(), style_change_event_time, is_valid);
  }
}

void HTMLElement::InvalidateLayoutBoxesFromNodeAndAncestors() {
  layout_boxes_.reset();
  Node::InvalidateLayoutBoxesFromNodeAndAncestors();
}

void HTMLElement::InvalidateLayoutBoxesFromNodeAndDescendants() {
  layout_boxes_.reset();
  Node::InvalidateLayoutBoxesFromNodeAndDescendants();
}

HTMLElement::HTMLElement(Document* document, base::Token tag_name)
    : Element(document, tag_name),
      style_(new cssom::CSSStyleDeclaration(
          document->html_element_context()->css_parser())),
      computed_style_valid_(false),
      computed_style_state_(new cssom::ComputedStyleState()),
      ALLOW_THIS_IN_INITIALIZER_LIST(
          transitions_adapter_(new DOMAnimatable(this))),
      css_transitions_(&transitions_adapter_),
      ALLOW_THIS_IN_INITIALIZER_LIST(
          animations_adapter_(new DOMAnimatable(this))),
      css_animations_(&animations_adapter_),
      matching_rules_valid_(false) {
  computed_style_state_->set_animations(animations());
  style_->set_mutation_observer(this);
}

HTMLElement::~HTMLElement() {}

void HTMLElement::OnInsertBefore(
    const scoped_refptr<Node>& /* new_child */,
    const scoped_refptr<Node>& /* reference_child */) {
  InvalidateMatchingRules();
}

void HTMLElement::OnRemoveChild(const scoped_refptr<Node>& /* node */) {
  InvalidateMatchingRules();
}

void HTMLElement::OnSetAttribute(const std::string& name,
                                 const std::string& /* value */) {
  if (name == "class" || name == "id") {
    InvalidateMatchingRules();
  }
}

void HTMLElement::OnRemoveAttribute(const std::string& name) {
  if (name == "class" || name == "id") {
    InvalidateMatchingRules();
  }
}

void HTMLElement::UpdateCachedBackgroundImagesFromComputedStyle() {
  scoped_refptr<cssom::PropertyValue> background_image =
      computed_style()->background_image();
  // Don't fetch or cache the image if the display of this element is turned
  // off.
  if (computed_style()->display() != cssom::KeywordValue::GetNone()) {
    cssom::PropertyListValue* property_list_value =
        base::polymorphic_downcast<cssom::PropertyListValue*>(
            background_image.get());

    loader::image::CachedImageReferenceVector cached_images;
    for (size_t i = 0; i < property_list_value->value().size(); ++i) {
      // Skip this image if it is not an absolute URL.
      if (property_list_value->value()[i]->GetTypeId() !=
          base::GetTypeId<cssom::AbsoluteURLValue>()) {
        continue;
      }

      cssom::AbsoluteURLValue* absolute_url =
          base::polymorphic_downcast<cssom::AbsoluteURLValue*>(
              property_list_value->value()[i].get());
      if (absolute_url->value().is_valid()) {
        scoped_refptr<loader::image::CachedImage> cached_image =
            html_element_context()->image_cache()->CreateCachedResource(
                absolute_url->value());
        base::Closure loaded_callback = base::Bind(
            &HTMLElement::OnBackgroundImageLoaded, base::Unretained(this));
        cached_images.push_back(
            new loader::image::CachedImageReferenceWithCallbacks(
                cached_image, loaded_callback, base::Closure()));
      }
    }

    cached_background_images_ = cached_images.Pass();
  } else {
    // Clear the previous cached background image if the display is "none".
    cached_background_images_.clear();
  }
}

void HTMLElement::OnBackgroundImageLoaded() {
  node_document()->RecordMutation();
}

bool HTMLElement::IsRootElement() {
  // The html element represents the root of an HTML document.
  //   https://www.w3.org/TR/2014/REC-html5-20141028/semantics.html#the-root-element
  return AsHTMLHtmlElement() != NULL;
}

}  // namespace dom
}  // namespace cobalt
