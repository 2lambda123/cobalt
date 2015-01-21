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

#include "cobalt/dom/attr.h"
#include "cobalt/dom/dom_stats.h"
#include "cobalt/dom/element.h"
#include "cobalt/dom/testing/gtest_workarounds.h"
#include "cobalt/dom/testing/html_collection_testing.h"
#include "cobalt/dom/testing/parent_node_testing.h"
#include "cobalt/dom/text.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace cobalt {
namespace dom {

//////////////////////////////////////////////////////////////////////////
// DocumentTest
//////////////////////////////////////////////////////////////////////////

class DocumentTest : public ::testing::Test {
 protected:
  DocumentTest() : html_element_factory_(NULL, NULL) {}
  ~DocumentTest() OVERRIDE {}

  // testing::Test:
  void SetUp() OVERRIDE;
  void TearDown() OVERRIDE;

  HTMLElementFactory html_element_factory_;
};

void DocumentTest::SetUp() {
  EXPECT_TRUE(DOMStats::GetInstance()->CheckNoLeaks());
}

void DocumentTest::TearDown() {
  EXPECT_TRUE(DOMStats::GetInstance()->CheckNoLeaks());
}

//////////////////////////////////////////////////////////////////////////
// Test cases
//////////////////////////////////////////////////////////////////////////

TEST_F(DocumentTest, Create) {
  scoped_refptr<Document> document = Document::Create(&html_element_factory_);
  ASSERT_NE(NULL, document);

  EXPECT_EQ(Node::kDocumentNode, document->node_type());
  EXPECT_EQ("#document", document->node_name());

  GURL url("http://a valid url");
  document = Document::CreateWithURL(&html_element_factory_, url);
  EXPECT_EQ(url.spec(), document->url());
  EXPECT_EQ(url.spec(), document->document_uri());
  EXPECT_EQ(url, document->url_as_gurl());
}

TEST_F(DocumentTest, CreateAttribute) {
  scoped_refptr<Document> document = Document::Create(&html_element_factory_);
  scoped_refptr<Attr> attribute = document->CreateAttribute("name", "value");

  EXPECT_EQ("name", attribute->name());
  EXPECT_EQ("name", attribute->node_name());
  EXPECT_EQ("value", attribute->value());
}

TEST_F(DocumentTest, CreateElement) {
  scoped_refptr<Document> document = Document::Create(&html_element_factory_);
  scoped_refptr<Element> element = document->CreateElement();

  EXPECT_EQ(Node::kElementNode, element->node_type());
  EXPECT_EQ("#element", element->node_name());

  // Make sure that the element is not attached to anything.
  EXPECT_EQ(NULL, element->owner_document());
  EXPECT_EQ(NULL, element->parent_node());
  EXPECT_EQ(NULL, element->first_child());
  EXPECT_EQ(NULL, element->last_child());
}

TEST_F(DocumentTest, CreateTextNode) {
  scoped_refptr<Document> document = Document::Create(&html_element_factory_);
  scoped_refptr<Text> text = document->CreateTextNode("test_text");

  EXPECT_EQ(Node::kTextNode, text->node_type());
  EXPECT_EQ("#text", text->node_name());
  EXPECT_EQ("test_text", text->text_content());

  // Make sure that the text is not attached to anything.
  EXPECT_EQ(NULL, text->owner_document());
}

TEST_F(DocumentTest, ParentNodeAllExceptChilden) {
  scoped_refptr<Document> root = Document::Create(&html_element_factory_);
  testing::TestParentNodeAllExceptChilden(root);
}

TEST_F(DocumentTest, ParentNodeChildren) {
  scoped_refptr<Document> root = Document::Create(&html_element_factory_);
  testing::TestParentNodeChildren(root);
}

TEST_F(DocumentTest, GetElementsByClassName) {
  scoped_refptr<Document> root = Document::Create(&html_element_factory_);
  testing::TestGetElementsByClassName(root);
}

TEST_F(DocumentTest, GetElementsByTagName) {
  scoped_refptr<Document> root = Document::Create(&html_element_factory_);
  testing::TestGetElementsByTagName(root);
}

TEST_F(DocumentTest, GetElementById) {
  scoped_refptr<Document> root = Document::Create(&html_element_factory_);

  // Construct a tree:
  // root
  //   a1
  //     b1
  //       c1
  //   a2
  //     d1
  scoped_refptr<Node> a1 = root->AppendChild(Element::Create());
  scoped_refptr<Node> a2 = root->AppendChild(Element::Create());
  scoped_refptr<Node> b1 = a1->AppendChild(Element::Create());
  scoped_refptr<Node> c1 = b1->AppendChild(Element::Create());
  scoped_refptr<Node> d1 = a2->AppendChild(Element::Create());

  EXPECT_EQ(NULL, root->GetElementById("id"));

  d1->AsElement()->set_id("id");
  EXPECT_EQ(d1, root->GetElementById("id"));

  c1->AsElement()->set_id("id");
  EXPECT_EQ(c1, root->GetElementById("id"));

  root->RemoveChild(a1);
  EXPECT_EQ(d1, root->GetElementById("id"));
}

TEST_F(DocumentTest, OwnerDocument) {
  // Construct a tree:
  // document
  //   element1
  //     element2
  scoped_refptr<Document> document = Document::Create(&html_element_factory_);
  scoped_refptr<Node> element1 = Element::Create();
  scoped_refptr<Node> element2 = Element::Create();

  EXPECT_EQ(NULL, document->owner_document());
  EXPECT_EQ(NULL, element1->owner_document());

  element1->AppendChild(element2);
  document->AppendChild(element1);
  EXPECT_EQ(NULL, document->owner_document());
  EXPECT_EQ(document, element1->owner_document());
  EXPECT_EQ(document, element2->owner_document());

  document->RemoveChild(element1);
  EXPECT_EQ(NULL, element1->owner_document());
  EXPECT_EQ(NULL, element2->owner_document());
}

}  // namespace dom
}  // namespace cobalt
