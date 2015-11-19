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

#include "cobalt/dom/document.h"

#include <vector>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/compiler_specific.h"
#include "base/debug/trace_event.h"
#include "base/message_loop.h"
#include "base/string_util.h"
#include "cobalt/cssom/css_media_rule.h"
#include "cobalt/cssom/css_rule.h"
#include "cobalt/cssom/css_rule_list.h"
#include "cobalt/cssom/css_style_rule.h"
#include "cobalt/cssom/css_style_sheet.h"
#include "cobalt/dom/attr.h"
#include "cobalt/dom/benchmark_stat_names.h"
#include "cobalt/dom/csp_delegate.h"
#include "cobalt/dom/dom_exception.h"
#include "cobalt/dom/dom_implementation.h"
#include "cobalt/dom/element.h"
#include "cobalt/dom/font_face_cache.h"
#include "cobalt/dom/font_face_cache_updater.h"
#include "cobalt/dom/html_body_element.h"
#include "cobalt/dom/html_collection.h"
#include "cobalt/dom/html_element.h"
#include "cobalt/dom/html_element_context.h"
#include "cobalt/dom/html_element_factory.h"
#include "cobalt/dom/html_head_element.h"
#include "cobalt/dom/html_html_element.h"
#include "cobalt/dom/location.h"
#include "cobalt/dom/named_node_map.h"
#include "cobalt/dom/node_descendants_iterator.h"
#include "cobalt/dom/node_list.h"
#include "cobalt/dom/rule_matching.h"
#include "cobalt/dom/text.h"
#include "cobalt/dom/ui_event.h"

namespace cobalt {
namespace dom {

Document::Document(HTMLElementContext* html_element_context,
                   const Options& options)
    : ALLOW_THIS_IN_INITIALIZER_LIST(Node(this)),
      html_element_context_(html_element_context),
      implementation_(new DOMImplementation()),
      location_(new Location(options.url)),
      url_(options.url),
      ALLOW_THIS_IN_INITIALIZER_LIST(csp_delegate_(new CSPDelegate(this))),
      ALLOW_THIS_IN_INITIALIZER_LIST(
          style_sheets_(new cssom::StyleSheetList(this))),
      ALLOW_THIS_IN_INITIALIZER_LIST(font_face_cache_(new FontFaceCache(
          html_element_context_ ? html_element_context_->remote_font_cache()
                                : NULL,
          base::Bind(&Document::OnFontLoadEvent, base::Unretained(this)),
          csp_delegate_.get()))),
      loading_counter_(0),
      should_dispatch_load_event_(true),
      is_selector_tree_dirty_(true),
      is_rule_matching_result_dirty_(true),
      is_computed_style_dirty_(true),
      are_font_faces_dirty_(true),
      navigation_start_clock_(options.navigation_start_clock),
      ALLOW_THIS_IN_INITIALIZER_LIST(
          default_timeline_(new DocumentTimeline(this, 0))) {
  DCHECK(url_.is_empty() || url_.is_valid());

#if defined(ENABLE_PARTIAL_LAYOUT_CONTROL)
  partial_layout_is_enabled_ = true;
#endif  // defined(ENABLE_PARTIAL_LAYOUT_CONTROL)

  // Sample the timeline upon initialization.
  SampleTimelineTime();

  // Call OnInsertedIntoDocument() immediately to ensure that the Document
  // object itself is considered to be "in the document".
  OnInsertedIntoDocument();
}

std::string Document::node_name() const {
  static const char kDocumentName[] = "#document";
  return kDocumentName;
}

scoped_refptr<Element> Document::document_element() {
  return first_element_child();
}

std::string Document::title() const {
  const char kTitleTag[] = "title";
  if (head()) {
    scoped_refptr<HTMLCollection> collection =
        head()->GetElementsByTagName(kTitleTag);
    if (collection->length() > 0) {
      return collection->Item(0)->text_content().value_or("");
    }
  }
  return "";
}

scoped_refptr<DOMImplementation> Document::implementation() {
  return implementation_;
}

scoped_refptr<HTMLCollection> Document::GetElementsByTagName(
    const std::string& local_name) const {
  return HTMLCollection::CreateWithElementsByTagName(this, local_name);
}

scoped_refptr<HTMLCollection> Document::GetElementsByClassName(
    const std::string& class_names) const {
  return HTMLCollection::CreateWithElementsByClassName(this, class_names);
}

scoped_refptr<Element> Document::CreateElement(const std::string& local_name) {
  if (IsXMLDocument()) {
    return new Element(this, local_name);
  } else {
    std::string lower_local_name = local_name;
    StringToLowerASCII(&lower_local_name);
    DCHECK(html_element_context_);
    DCHECK(html_element_context_->html_element_factory());
    return html_element_context_->html_element_factory()->CreateHTMLElement(
        this, lower_local_name);
  }
}

scoped_refptr<Element> Document::CreateElementNS(
    const std::string& namespace_uri, const std::string& local_name) {
  // TODO(***REMOVED***): Implement namespaces, if we actually need this.
  NOTIMPLEMENTED() << namespace_uri;
  return CreateElement(local_name);
}

scoped_refptr<Text> Document::CreateTextNode(const std::string& text) {
  return new Text(this, text);
}

scoped_refptr<Event> Document::CreateEvent(
    const std::string& interface_name,
    script::ExceptionState* exception_state) {
  // http://www.w3.org/TR/2015/WD-dom-20150428/#dom-document-createevent
  // The match of interface name is case-insensitive.
  if (strcasecmp(interface_name.c_str(), "event") == 0 ||
      strcasecmp(interface_name.c_str(), "events") == 0) {
    return new Event(Event::Uninitialized);
  } else if (strcasecmp(interface_name.c_str(), "uievent") == 0 ||
             strcasecmp(interface_name.c_str(), "uievents") == 0) {
    return new UIEvent(Event::Uninitialized);
  }

  DOMException::Raise(DOMException::kNotSupportedErr, exception_state);
  // Return value will be ignored.
  return NULL;
}

scoped_refptr<Element> Document::GetElementById(const std::string& id) const {
  NodeDescendantsIterator iterator(this);

  // TODO(***REMOVED***): Consider optimizing this method by replacing the linear
  // search with a constant time lookup.
  Node* child = iterator.First();
  while (child) {
    scoped_refptr<Element> element = child->AsElement();
    if (element && element->id() == id) {
      return element;
    }
    child = iterator.Next();
  }
  return NULL;
}

scoped_refptr<Location> Document::location() const { return location_; }

scoped_refptr<HTMLBodyElement> Document::body() const { return body_.get(); }

// Algorithm for set_body:
//   http://www.w3.org/TR/html5/dom.html#dom-document-body
void Document::set_body(const scoped_refptr<HTMLBodyElement>& value) {
  // 1. If the new value is not a body or frameset element, then throw a
  //    HierarchyRequestError exception and abort these steps.
  if (value->tag_name() != HTMLBodyElement::kTagName) {
    // TODO(***REMOVED***): Throw JS HierarchyRequestError.
    return;
  }

  // 2. Otherwise, if the new value is the same as the body element, do nothing.
  //    Abort these steps.
  scoped_refptr<HTMLBodyElement> current_body = body();
  if (current_body == value) return;

  // 3. Otherwise, if the body element is not null, then replace that element
  //    with the new value in the DOM, as if the root element's replaceChild()
  //    method had been called with the new value and the incumbent body element
  //    as its two arguments respectively, then abort these steps.
  // 4. Otherwise, if there is no root element, throw a HierarchyRequestError
  //    exception and abort these steps.
  // 5. Otherwise, the body element is null, but there's a root element. Append
  //    the new value to the root element.
  scoped_refptr<HTMLHtmlElement> current_html = html();
  if (!current_html) {
    // TODO(***REMOVED***): Throw JS HierarchyRequestError.
    return;
  }
  if (current_body) {
    current_html->ReplaceChild(value, current_body);
  } else {
    current_html->AppendChild(value);
  }
}

scoped_refptr<HTMLHeadElement> Document::head() const { return head_.get(); }

scoped_refptr<Element> Document::active_element() const {
  if (!active_element_) {
    return body();
  } else {
    return active_element_.get();
  }
}

scoped_refptr<Element> Document::QuerySelector(const std::string& selectors) {
  return QuerySelectorInternal(selectors, html_element_context_->css_parser());
}

scoped_refptr<NodeList> Document::QuerySelectorAll(
    const std::string& selectors) {
  return QuerySelectorAllInternal(selectors,
                                  html_element_context()->css_parser());
}

void Document::Accept(NodeVisitor* visitor) { visitor->Visit(this); }

void Document::Accept(ConstNodeVisitor* visitor) const { visitor->Visit(this); }

scoped_refptr<HTMLHtmlElement> Document::html() const { return html_.get(); }

void Document::SetBody(HTMLBodyElement* body) {
  if (body) {
    LOG_IF(ERROR, body_) << "Document contains more than one <body> tags.";
    body_ = base::AsWeakPtr(body);
  } else {
    body_.reset();
  }
}

void Document::SetHead(HTMLHeadElement* head) {
  if (head) {
    LOG_IF(ERROR, head_) << "Document contains more than one <head> tags.";
    head_ = base::AsWeakPtr(head);
  } else {
    head_.reset();
  }
}

void Document::SetHtml(HTMLHtmlElement* html) {
  if (html) {
    LOG_IF(ERROR, html_) << "Document contains more than one <html> tags.";
    html_ = base::AsWeakPtr(html);
  } else {
    html_.reset();
  }
}

void Document::SetActiveElement(Element* active_element) {
  if (active_element) {
    active_element_ = base::AsWeakPtr(active_element);
  } else {
    active_element_.reset();
  }
}

void Document::IncreaseLoadingCounter() { ++loading_counter_; }

void Document::DecreaseLoadingCounterAndMaybeDispatchLoadEvent(
    bool load_succeeded) {
  if (!load_succeeded) {
    // If the document or any dependent script failed loading, load event
    // shouldn't be dispatched.
    should_dispatch_load_event_ = false;
  } else {
    DCHECK_GT(loading_counter_, 0);
    loading_counter_--;
    if (loading_counter_ == 0 && should_dispatch_load_event_) {
      DCHECK(MessageLoop::current());
      should_dispatch_load_event_ = false;

      MessageLoop::current()->PostTask(
          FROM_HERE, base::Bind(base::IgnoreResult(&Document::DispatchEvent),
                                base::AsWeakPtr<Document>(this),
                                make_scoped_refptr(new Event("load"))));

      // After all JavaScript OnLoad event handlers have executed, signal to any
      // Document observers know that a load event has occurred.
      MessageLoop::current()->PostTask(
          FROM_HERE, base::Bind(&Document::SignalOnLoadToObservers,
                                base::AsWeakPtr<Document>(this)));
    }
  }
}

void Document::AddObserver(DocumentObserver* observer) {
  observers_.AddObserver(observer);
}

void Document::RemoveObserver(DocumentObserver* observer) {
  observers_.RemoveObserver(observer);
}

void Document::SignalOnLoadToObservers() {
  FOR_EACH_OBSERVER(DocumentObserver, observers_, OnLoad());
}

void Document::RecordMutation() {
  TRACE_EVENT0("cobalt::dom", "Document::RecordMutation()");

  FOR_EACH_OBSERVER(DocumentObserver, observers_, OnMutation());
}

void Document::OnCSSMutation() {
  // Something in the document's CSS rules has been modified, but we don't know
  // what, so set the flag indicating that rule matching needs to be done.
  is_selector_tree_dirty_ = true;
  is_rule_matching_result_dirty_ = true;
  is_computed_style_dirty_ = true;
  are_font_faces_dirty_ = true;

  RecordMutation();
}

void Document::OnDOMMutation() {
  // Something in the document's DOM has been modified, but we don't know what,
  // so set the flag indicating that rule matching needs to be done.
  is_rule_matching_result_dirty_ = true;
  is_computed_style_dirty_ = true;

  RecordMutation();
}

void Document::OnFontLoadEvent() {
  InvalidateLayoutBoxesFromNodeAndDescendants();
  RecordMutation();
}

void Document::OnElementInlineStyleMutation() {
  is_computed_style_dirty_ = true;

  RecordMutation();
}

namespace {

void RemoveRulesFromCSSRuleListFromSelectorTree(
    cssom::SelectorTree* selector_tree,
    const scoped_refptr<cssom::CSSRuleList>& css_rule_list) {
  for (unsigned int i = 0; i < css_rule_list->length(); ++i) {
    cssom::CSSRule* rule = css_rule_list->Item(i);

    cssom::CSSStyleRule* css_style_rule = rule->AsCSSStyleRule();
    if (css_style_rule && css_style_rule->added_to_selector_tree()) {
      selector_tree->RemoveRule(css_style_rule);
      css_style_rule->set_added_to_selector_tree(false);
    }

    cssom::CSSMediaRule* css_media_rule = rule->AsCSSMediaRule();
    if (css_media_rule) {
      RemoveRulesFromCSSRuleListFromSelectorTree(selector_tree,
                                                 css_media_rule->css_rules());
    }
  }
}

void AppendRulesFromCSSRuleListToSelectorTree(
    cssom::SelectorTree* selector_tree,
    const scoped_refptr<cssom::CSSRuleList>& css_rule_list) {
  for (unsigned int i = 0; i < css_rule_list->length(); ++i) {
    cssom::CSSRule* rule = css_rule_list->Item(i);

    cssom::CSSStyleRule* css_style_rule = rule->AsCSSStyleRule();
    if (css_style_rule && !css_style_rule->added_to_selector_tree()) {
      selector_tree->AppendRule(css_style_rule);
      css_style_rule->set_added_to_selector_tree(true);
    }

    cssom::CSSMediaRule* css_media_rule = rule->AsCSSMediaRule();
    if (css_media_rule) {
      if (css_media_rule->condition_value()) {
        AppendRulesFromCSSRuleListToSelectorTree(selector_tree,
                                                 css_media_rule->css_rules());
      } else {
        RemoveRulesFromCSSRuleListFromSelectorTree(selector_tree,
                                                   css_media_rule->css_rules());
      }
    }
  }
}

void UpdateSelectorTreeFromCSSStyleSheet(
    cssom::SelectorTree* selector_tree,
    const scoped_refptr<cssom::CSSStyleSheet>& style_sheet) {
  AppendRulesFromCSSRuleListToSelectorTree(selector_tree,
                                           style_sheet->css_rules());
}

}  // namespace

void Document::UpdateMatchingRules(
    const scoped_refptr<cssom::CSSStyleDeclarationData>& root_computed_style,
    const scoped_refptr<cssom::CSSStyleSheet>& user_agent_style_sheet) {
  TRACE_EVENT0("cobalt::dom", "Document::UpdateMatchingRules()");
  DCHECK(html());

  if (is_selector_tree_dirty_) {
    TRACE_EVENT0("cobalt::dom", kBenchmarkStatUpdateSelectorTree);

    scoped_refptr<cssom::PropertyValue> width(root_computed_style->width());
    scoped_refptr<cssom::PropertyValue> height(root_computed_style->height());

    if (user_agent_style_sheet) {
      user_agent_style_sheet->EvaluateMediaRules(width, height);
      UpdateSelectorTreeFromCSSStyleSheet(&selector_tree_,
                                          user_agent_style_sheet);
    }

    for (unsigned int style_sheet_index = 0;
         style_sheet_index < style_sheets_->length(); ++style_sheet_index) {
      scoped_refptr<cssom::CSSStyleSheet> css_style_sheet =
          style_sheets_->Item(style_sheet_index)->AsCSSStyleSheet();

      css_style_sheet->EvaluateMediaRules(width, height);
      UpdateSelectorTreeFromCSSStyleSheet(&selector_tree_, css_style_sheet);
    }

    html()->InvalidateMatchingRules();
    is_selector_tree_dirty_ = false;
  }

  if (is_rule_matching_result_dirty_) {
    TRACE_EVENT0("cobalt::dom", kBenchmarkStatUpdateMatchingRules);

    UpdateMatchingRulesUsingSelectorTree(html(), &selector_tree_);
    is_rule_matching_result_dirty_ = false;
  }
}

void Document::UpdateComputedStyles(
    const scoped_refptr<cssom::CSSStyleDeclarationData>& root_computed_style,
    const scoped_refptr<cssom::CSSStyleSheet>& user_agent_style_sheet) {
  TRACE_EVENT0("cobalt::dom", "Document::UpdateComputedStyles()");

  UpdateMatchingRules(root_computed_style, user_agent_style_sheet);

  if (is_computed_style_dirty_) {
    // Determine the official time that this style change event took place. This
    // is needed (as opposed to repeatedly calling base::Time::Now()) because
    // all animations that may be triggered here must start at the exact same
    // time if they were triggered in the same style change event.
    //   http://www.w3.org/TR/css3-transitions/#starting
    base::TimeDelta style_change_event_time =
        base::TimeDelta::FromMillisecondsD(*default_timeline_->current_time());

    TRACE_EVENT0("cobalt::layout", kBenchmarkStatUpdateComputedStyles);
    html()->UpdateComputedStyleRecursively(root_computed_style,
                                           style_change_event_time, true);

    is_computed_style_dirty_ = false;
  }

  UpdateFontFaces(user_agent_style_sheet);
}

void Document::UpdateFontFaces(
    const scoped_refptr<cssom::CSSStyleSheet>& user_agent_style_sheet) {
  if (are_font_faces_dirty_) {
    TRACE_EVENT0("cobalt::layout", kBenchmarkStatUpdateFontFaces);
    FontFaceCacheUpdater font_face_updater(url_, font_face_cache_.get());
    font_face_updater.ProcessCSSStyleSheet(user_agent_style_sheet);
    font_face_updater.ProcessStyleSheetList(style_sheets());
    are_font_faces_dirty_ = false;
  }
}

void Document::SampleTimelineTime() { default_timeline_->Sample(); }

#if defined(ENABLE_PARTIAL_LAYOUT_CONTROL)
void Document::SetPartialLayout(const std::string& mode_string) {
  std::vector<std::string> mode_tokens;
  Tokenize(mode_string, ",", &mode_tokens);
  for (std::vector<std::string>::iterator mode_token_iterator =
           mode_tokens.begin();
       mode_token_iterator != mode_tokens.end(); ++mode_token_iterator) {
    const std::string& mode_token = *mode_token_iterator;
    if (mode_token == "wipe") {
      InvalidateLayoutBoxesFromNodeAndDescendants();
      DLOG(INFO) << "Partial Layout state wiped";
    } else if (mode_token == "off") {
      partial_layout_is_enabled_ = false;
      DLOG(INFO) << "Partial Layout mode turned off";
    } else if (mode_token == "on") {
      partial_layout_is_enabled_ = true;
      DLOG(INFO) << "Partial Layout mode turned on";
    } else if (mode_token == "undefined") {
      DLOG(INFO) << "Partial Layout mode is currently "
                 << (partial_layout_is_enabled_ ? "on" : "off");
    } else {
      DLOG(WARNING) << "Partial Layout mode \"" << mode_string
                    << "\" not recognized.";
    }
  }
}
#endif  // defined(ENABLE_PARTIAL_LAYOUT_CONTROL)

Document::~Document() {
  // Ensure that all outstanding weak ptrs become invalid.
  // Some objects that will be released while this destructor runs may
  // have weak ptrs to |this|.
  InvalidateWeakPtrs();
}

}  // namespace dom
}  // namespace cobalt
