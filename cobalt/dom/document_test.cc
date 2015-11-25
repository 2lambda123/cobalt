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

#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/css_parser/parser.h"
#include "cobalt/cssom/css_style_sheet.h"
#include "cobalt/dom/attr.h"
#include "cobalt/dom/dom_exception.h"
#include "cobalt/dom/dom_implementation.h"
#include "cobalt/dom/element.h"
#include "cobalt/dom/html_element_context.h"
#include "cobalt/dom/html_style_element.h"
#include "cobalt/dom/location.h"
#include "cobalt/dom/node_list.h"
#include "cobalt/dom/stats.h"
#include "cobalt/dom/testing/gtest_workarounds.h"
#include "cobalt/dom/testing/html_collection_testing.h"
#include "cobalt/dom/text.h"
#include "cobalt/script/testing/mock_exception_state.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace cobalt {
namespace dom {
namespace {

using ::testing::SaveArg;
using ::testing::StrictMock;
using ::testing::_;
using script::testing::MockExceptionState;

//////////////////////////////////////////////////////////////////////////
// DocumentTest
//////////////////////////////////////////////////////////////////////////

class DocumentTest : public ::testing::Test {
 protected:
  DocumentTest();
  ~DocumentTest() OVERRIDE;

  scoped_ptr<css_parser::Parser> css_parser_;
  HTMLElementContext html_element_context_;
};

DocumentTest::DocumentTest()
    : css_parser_(css_parser::Parser::Create()),
      html_element_context_(NULL, css_parser_.get(), NULL, NULL, NULL, NULL,
                            NULL, NULL, NULL, "") {
  EXPECT_TRUE(Stats::GetInstance()->CheckNoLeaks());
}

DocumentTest::~DocumentTest() {
  EXPECT_TRUE(Stats::GetInstance()->CheckNoLeaks());
}

//////////////////////////////////////////////////////////////////////////
// Test cases
//////////////////////////////////////////////////////////////////////////

TEST_F(DocumentTest, Create) {
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());
  ASSERT_TRUE(document);

  EXPECT_EQ(Node::kDocumentNode, document->node_type());
  EXPECT_EQ("#document", document->node_name());

  GURL url("http://a valid url");
  document = new Document(&html_element_context_, Document::Options(url));
  EXPECT_EQ(url.spec(), document->url());
  EXPECT_EQ(url.spec(), document->document_uri());
  EXPECT_EQ(url, document->url_as_gurl());
}

TEST_F(DocumentTest, Duplicate) {
  GURL url("http://a valid url");
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options(url));
  scoped_refptr<Document> new_document = document->Duplicate()->AsDocument();
  ASSERT_TRUE(new_document);
  EXPECT_FALSE(new_document->IsXMLDocument());
  EXPECT_EQ(url.spec(), new_document->url());
}

TEST_F(DocumentTest, IsNotXMLDocument) {
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());
  EXPECT_FALSE(document->IsXMLDocument());
}

TEST_F(DocumentTest, DocumentElement) {
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());
  EXPECT_EQ(NULL, document->document_element());

  scoped_refptr<Text> text = new Text(document, "test_text");
  scoped_refptr<Element> element = new Element(document);
  document->AppendChild(text);
  document->AppendChild(element);
  EXPECT_EQ(element, document->document_element());
}

TEST_F(DocumentTest, CreateElement) {
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());
  scoped_refptr<Element> element = document->CreateElement("element");

  EXPECT_EQ(Node::kElementNode, element->node_type());
  EXPECT_EQ("element", element->node_name());

  EXPECT_EQ(document, element->owner_document());
  EXPECT_EQ(NULL, element->parent_node());
  EXPECT_EQ(NULL, element->first_child());
  EXPECT_EQ(NULL, element->last_child());

  element = document->CreateElement("ELEMENT");
  EXPECT_EQ("element", element->node_name());
}

TEST_F(DocumentTest, CreateTextNode) {
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());
  scoped_refptr<Text> text = document->CreateTextNode("test_text");

  EXPECT_EQ(Node::kTextNode, text->node_type());
  EXPECT_EQ("#text", text->node_name());
  EXPECT_EQ("test_text", text->data());
  EXPECT_EQ(document, text->owner_document());
}

TEST_F(DocumentTest, CreateEvent) {
  StrictMock<MockExceptionState> exception_state;
  scoped_refptr<script::ScriptException> exception;
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());
  // Create an Event, the name is case insensitive.
  scoped_refptr<Event> event = document->CreateEvent("EvEnT", &exception_state);

  EXPECT_TRUE(event);
  EXPECT_FALSE(event->initialized_flag());

  EXPECT_CALL(exception_state, SetException(_))
      .WillOnce(SaveArg<0>(&exception));
  event = document->CreateEvent("Event Not Supported", &exception_state);

  EXPECT_FALSE(event);
  ASSERT_TRUE(exception);
  EXPECT_EQ(DOMException::kNotSupportedErr,
            base::polymorphic_downcast<DOMException*>(exception.get())->code());
}

TEST_F(DocumentTest, GetElementsByClassName) {
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());
  testing::TestGetElementsByClassName(document);
}

TEST_F(DocumentTest, GetElementsByTagName) {
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());
  testing::TestGetElementsByTagName(document);
}

TEST_F(DocumentTest, GetElementById) {
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());

  // Construct a tree:
  // document
  //   a1
  //     b1
  //       c1
  //   a2
  //     d1
  scoped_refptr<Node> a1 = document->AppendChild(new Element(document));
  scoped_refptr<Node> a2 = document->AppendChild(new Element(document));
  scoped_refptr<Node> b1 = a1->AppendChild(new Element(document));
  scoped_refptr<Node> c1 = b1->AppendChild(new Element(document));
  scoped_refptr<Node> d1 = a2->AppendChild(new Element(document));

  EXPECT_EQ(NULL, document->GetElementById("id"));

  d1->AsElement()->set_id("id");
  EXPECT_EQ(d1, document->GetElementById("id"));

  c1->AsElement()->set_id("id");
  EXPECT_EQ(c1, document->GetElementById("id"));

  document->RemoveChild(a1);
  EXPECT_EQ(d1, document->GetElementById("id"));
}

TEST_F(DocumentTest, OwnerDocument) {
  // Construct a tree:
  // document
  //   element1
  //     element2
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());
  scoped_refptr<Node> element1 = new Element(document);
  scoped_refptr<Node> element2 = new Element(document);

  EXPECT_EQ(NULL, document->owner_document());
  EXPECT_EQ(document, element1->owner_document());

  element1->AppendChild(element2);
  document->AppendChild(element1);
  EXPECT_EQ(NULL, document->owner_document());
  EXPECT_EQ(document, element1->owner_document());
  EXPECT_EQ(document, element2->owner_document());

  document->RemoveChild(element1);
  EXPECT_EQ(document, element1->owner_document());
  EXPECT_EQ(document, element2->owner_document());
}

TEST_F(DocumentTest, Implementation) {
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());
  EXPECT_TRUE(document->implementation());
}

TEST_F(DocumentTest, Location) {
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());
  EXPECT_TRUE(document->location());
}

TEST_F(DocumentTest, StyleSheets) {
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());

  scoped_refptr<HTMLElement> element1 =
      html_element_context_.html_element_factory()->CreateHTMLElement(
          document, HTMLStyleElement::kTagName);
  element1->set_text_content(std::string("body { background-color: #D3D3D3 }"));
  document->AppendChild(element1);

  scoped_refptr<HTMLElement> element2 =
      html_element_context_.html_element_factory()->CreateHTMLElement(
          document, HTMLStyleElement::kTagName);
  element2->set_text_content(std::string("h1 { color: #00F }"));
  document->AppendChild(element2);

  scoped_refptr<HTMLElement> element3 =
      html_element_context_.html_element_factory()->CreateHTMLElement(
          document, HTMLStyleElement::kTagName);
  element3->set_text_content(std::string("p { color: #008000 }"));
  document->AppendChild(element3);

  EXPECT_TRUE(document->style_sheets());
  EXPECT_EQ(3, document->style_sheets()->length());
  EXPECT_TRUE(document->style_sheets()->Item(0));
  EXPECT_TRUE(document->style_sheets()->Item(1));
  EXPECT_TRUE(document->style_sheets()->Item(2));

  // Each style sheet shoult represent the one from the corresponding style
  // element.
  EXPECT_EQ(document->style_sheets()->Item(0),
            element1->AsHTMLStyleElement()->sheet());
  EXPECT_EQ(document->style_sheets()->Item(1),
            element2->AsHTMLStyleElement()->sheet());
  EXPECT_EQ(document->style_sheets()->Item(2),
            element3->AsHTMLStyleElement()->sheet());

  // Each style sheet should be unique.
  EXPECT_NE(document->style_sheets()->Item(0),
            document->style_sheets()->Item(1));
  EXPECT_NE(document->style_sheets()->Item(0),
            document->style_sheets()->Item(2));
  EXPECT_NE(document->style_sheets()->Item(1),
            document->style_sheets()->Item(2));
}

TEST_F(DocumentTest, QuerySelector) {
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());
  document->AppendChild(document->CreateElement("div"));
  document->AppendChild(document->CreateElement("div"));
  EXPECT_FALSE(document->QuerySelector("span"));
  // QuerySelector should return first matching child.
  EXPECT_EQ(document->first_element_child(), document->QuerySelector("div"));
}

TEST_F(DocumentTest, QuerySelectorAll) {
  scoped_refptr<Document> document =
      new Document(&html_element_context_, Document::Options());
  document->AppendChild(document->CreateElement("div"));
  document->AppendChild(document->CreateElement("div"));
  scoped_refptr<NodeList> result = document->QuerySelectorAll("span");
  EXPECT_EQ(0, result->length());

  result = document->QuerySelectorAll("div");
  EXPECT_EQ(2, result->length());
  EXPECT_EQ(document->first_element_child(), result->Item(0));
  EXPECT_EQ(document->last_element_child(), result->Item(1));
}

}  // namespace
}  // namespace dom
}  // namespace cobalt
