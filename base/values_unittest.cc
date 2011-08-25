// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "base/memory/scoped_ptr.h"
#include "base/string16.h"
#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace base {

TEST(ValuesTest, Basic) {
  // Test basic dictionary getting/setting
  DictionaryValue settings;
  std::string homepage = "http://google.com";
  ASSERT_FALSE(settings.GetString("global.homepage", &homepage));
  ASSERT_EQ(std::string("http://google.com"), homepage);

  ASSERT_FALSE(settings.Get("global", NULL));
  settings.Set("global", Value::CreateBooleanValue(true));
  ASSERT_TRUE(settings.Get("global", NULL));
  settings.SetString("global.homepage", "http://scurvy.com");
  ASSERT_TRUE(settings.Get("global", NULL));
  homepage = "http://google.com";
  ASSERT_TRUE(settings.GetString("global.homepage", &homepage));
  ASSERT_EQ(std::string("http://scurvy.com"), homepage);

  // Test storing a dictionary in a list.
  ListValue* toolbar_bookmarks;
  ASSERT_FALSE(
    settings.GetList("global.toolbar.bookmarks", &toolbar_bookmarks));

  toolbar_bookmarks = new ListValue;
  settings.Set("global.toolbar.bookmarks", toolbar_bookmarks);
  ASSERT_TRUE(settings.GetList("global.toolbar.bookmarks", &toolbar_bookmarks));

  DictionaryValue* new_bookmark = new DictionaryValue;
  new_bookmark->SetString("name", "Froogle");
  new_bookmark->SetString("url", "http://froogle.com");
  toolbar_bookmarks->Append(new_bookmark);

  ListValue* bookmark_list;
  ASSERT_TRUE(settings.GetList("global.toolbar.bookmarks", &bookmark_list));
  DictionaryValue* bookmark;
  ASSERT_EQ(1U, bookmark_list->GetSize());
  ASSERT_TRUE(bookmark_list->GetDictionary(0, &bookmark));
  std::string bookmark_name = "Unnamed";
  ASSERT_TRUE(bookmark->GetString("name", &bookmark_name));
  ASSERT_EQ(std::string("Froogle"), bookmark_name);
  std::string bookmark_url;
  ASSERT_TRUE(bookmark->GetString("url", &bookmark_url));
  ASSERT_EQ(std::string("http://froogle.com"), bookmark_url);
}

TEST(ValuesTest, List) {
  scoped_ptr<ListValue> mixed_list(new ListValue());
  mixed_list->Set(0, Value::CreateBooleanValue(true));
  mixed_list->Set(1, Value::CreateIntegerValue(42));
  mixed_list->Set(2, Value::CreateDoubleValue(88.8));
  mixed_list->Set(3, Value::CreateStringValue("foo"));
  ASSERT_EQ(4u, mixed_list->GetSize());

  Value *value = NULL;
  bool bool_value = false;
  int int_value = 0;
  double double_value = 0.0;
  std::string string_value;

  ASSERT_FALSE(mixed_list->Get(4, &value));

  ASSERT_FALSE(mixed_list->GetInteger(0, &int_value));
  ASSERT_EQ(0, int_value);
  ASSERT_FALSE(mixed_list->GetBoolean(1, &bool_value));
  ASSERT_FALSE(bool_value);
  ASSERT_FALSE(mixed_list->GetString(2, &string_value));
  ASSERT_EQ("", string_value);
  ASSERT_FALSE(mixed_list->GetInteger(2, &int_value));
  ASSERT_EQ(0, int_value);
  ASSERT_FALSE(mixed_list->GetBoolean(3, &bool_value));
  ASSERT_FALSE(bool_value);

  ASSERT_TRUE(mixed_list->GetBoolean(0, &bool_value));
  ASSERT_TRUE(bool_value);
  ASSERT_TRUE(mixed_list->GetInteger(1, &int_value));
  ASSERT_EQ(42, int_value);
  // implicit conversion from Integer to Double should be possible.
  ASSERT_TRUE(mixed_list->GetDouble(1, &double_value));
  ASSERT_EQ(42, double_value);
  ASSERT_TRUE(mixed_list->GetDouble(2, &double_value));
  ASSERT_EQ(88.8, double_value);
  ASSERT_TRUE(mixed_list->GetString(3, &string_value));
  ASSERT_EQ("foo", string_value);
}

TEST(ValuesTest, BinaryValue) {
  char* buffer = NULL;
  // Passing a null buffer pointer doesn't yield a BinaryValue
  scoped_ptr<BinaryValue> binary(BinaryValue::Create(buffer, 0));
  ASSERT_FALSE(binary.get());

  // If you want to represent an empty binary value, use a zero-length buffer.
  buffer = new char[1];
  ASSERT_TRUE(buffer);
  binary.reset(BinaryValue::Create(buffer, 0));
  ASSERT_TRUE(binary.get());
  ASSERT_TRUE(binary->GetBuffer());
  ASSERT_EQ(buffer, binary->GetBuffer());
  ASSERT_EQ(0U, binary->GetSize());

  // Test the common case of a non-empty buffer
  buffer = new char[15];
  binary.reset(BinaryValue::Create(buffer, 15));
  ASSERT_TRUE(binary.get());
  ASSERT_TRUE(binary->GetBuffer());
  ASSERT_EQ(buffer, binary->GetBuffer());
  ASSERT_EQ(15U, binary->GetSize());

  char stack_buffer[42];
  memset(stack_buffer, '!', 42);
  binary.reset(BinaryValue::CreateWithCopiedBuffer(stack_buffer, 42));
  ASSERT_TRUE(binary.get());
  ASSERT_TRUE(binary->GetBuffer());
  ASSERT_NE(stack_buffer, binary->GetBuffer());
  ASSERT_EQ(42U, binary->GetSize());
  ASSERT_EQ(0, memcmp(stack_buffer, binary->GetBuffer(), binary->GetSize()));
}

TEST(ValuesTest, StringValue) {
  // Test overloaded CreateStringValue.
  scoped_ptr<Value> narrow_value(Value::CreateStringValue("narrow"));
  ASSERT_TRUE(narrow_value.get());
  ASSERT_TRUE(narrow_value->IsType(Value::TYPE_STRING));
  scoped_ptr<Value> utf16_value(
      Value::CreateStringValue(ASCIIToUTF16("utf16")));
  ASSERT_TRUE(utf16_value.get());
  ASSERT_TRUE(utf16_value->IsType(Value::TYPE_STRING));

  // Test overloaded GetString.
  std::string narrow = "http://google.com";
  string16 utf16 = ASCIIToUTF16("http://google.com");
  ASSERT_TRUE(narrow_value->GetAsString(&narrow));
  ASSERT_TRUE(narrow_value->GetAsString(&utf16));
  ASSERT_EQ(std::string("narrow"), narrow);
  ASSERT_EQ(ASCIIToUTF16("narrow"), utf16);

  ASSERT_TRUE(utf16_value->GetAsString(&narrow));
  ASSERT_TRUE(utf16_value->GetAsString(&utf16));
  ASSERT_EQ(std::string("utf16"), narrow);
  ASSERT_EQ(ASCIIToUTF16("utf16"), utf16);
}

// This is a Value object that allows us to tell if it's been
// properly deleted by modifying the value of external flag on destruction.
class DeletionTestValue : public Value {
 public:
  explicit DeletionTestValue(bool* deletion_flag) : Value(TYPE_NULL) {
    Init(deletion_flag);  // Separate function so that we can use ASSERT_*
  }

  void Init(bool* deletion_flag) {
    ASSERT_TRUE(deletion_flag);
    deletion_flag_ = deletion_flag;
    *deletion_flag_ = false;
  }

  ~DeletionTestValue() {
    *deletion_flag_ = true;
  }

 private:
  bool* deletion_flag_;
};

TEST(ValuesTest, ListDeletion) {
  bool deletion_flag = true;

  {
    ListValue list;
    list.Append(new DeletionTestValue(&deletion_flag));
    EXPECT_FALSE(deletion_flag);
  }
  EXPECT_TRUE(deletion_flag);

  {
    ListValue list;
    list.Append(new DeletionTestValue(&deletion_flag));
    EXPECT_FALSE(deletion_flag);
    list.Clear();
    EXPECT_TRUE(deletion_flag);
  }

  {
    ListValue list;
    list.Append(new DeletionTestValue(&deletion_flag));
    EXPECT_FALSE(deletion_flag);
    EXPECT_TRUE(list.Set(0, Value::CreateNullValue()));
    EXPECT_TRUE(deletion_flag);
  }
}

TEST(ValuesTest, ListRemoval) {
  bool deletion_flag = true;
  Value* removed_item = NULL;

  {
    ListValue list;
    list.Append(new DeletionTestValue(&deletion_flag));
    EXPECT_FALSE(deletion_flag);
    EXPECT_EQ(1U, list.GetSize());
    EXPECT_FALSE(list.Remove(std::numeric_limits<size_t>::max(),
                             &removed_item));
    EXPECT_FALSE(list.Remove(1, &removed_item));
    EXPECT_TRUE(list.Remove(0, &removed_item));
    ASSERT_TRUE(removed_item);
    EXPECT_EQ(0U, list.GetSize());
  }
  EXPECT_FALSE(deletion_flag);
  delete removed_item;
  removed_item = NULL;
  EXPECT_TRUE(deletion_flag);

  {
    ListValue list;
    list.Append(new DeletionTestValue(&deletion_flag));
    EXPECT_FALSE(deletion_flag);
    EXPECT_TRUE(list.Remove(0, NULL));
    EXPECT_TRUE(deletion_flag);
    EXPECT_EQ(0U, list.GetSize());
  }

  {
    ListValue list;
    DeletionTestValue* value = new DeletionTestValue(&deletion_flag);
    list.Append(value);
    EXPECT_FALSE(deletion_flag);
    size_t index = 0;
    list.Remove(*value, &index);
    EXPECT_EQ(0U, index);
    EXPECT_TRUE(deletion_flag);
    EXPECT_EQ(0U, list.GetSize());
  }
}

TEST(ValuesTest, DictionaryDeletion) {
  std::string key = "test";
  bool deletion_flag = true;

  {
    DictionaryValue dict;
    dict.Set(key, new DeletionTestValue(&deletion_flag));
    EXPECT_FALSE(deletion_flag);
  }
  EXPECT_TRUE(deletion_flag);

  {
    DictionaryValue dict;
    dict.Set(key, new DeletionTestValue(&deletion_flag));
    EXPECT_FALSE(deletion_flag);
    dict.Clear();
    EXPECT_TRUE(deletion_flag);
  }

  {
    DictionaryValue dict;
    dict.Set(key, new DeletionTestValue(&deletion_flag));
    EXPECT_FALSE(deletion_flag);
    dict.Set(key, Value::CreateNullValue());
    EXPECT_TRUE(deletion_flag);
  }
}

TEST(ValuesTest, DictionaryRemoval) {
  std::string key = "test";
  bool deletion_flag = true;
  Value* removed_item = NULL;

  {
    DictionaryValue dict;
    dict.Set(key, new DeletionTestValue(&deletion_flag));
    EXPECT_FALSE(deletion_flag);
    EXPECT_TRUE(dict.HasKey(key));
    EXPECT_FALSE(dict.Remove("absent key", &removed_item));
    EXPECT_TRUE(dict.Remove(key, &removed_item));
    EXPECT_FALSE(dict.HasKey(key));
    ASSERT_TRUE(removed_item);
  }
  EXPECT_FALSE(deletion_flag);
  delete removed_item;
  removed_item = NULL;
  EXPECT_TRUE(deletion_flag);

  {
    DictionaryValue dict;
    dict.Set(key, new DeletionTestValue(&deletion_flag));
    EXPECT_FALSE(deletion_flag);
    EXPECT_TRUE(dict.HasKey(key));
    EXPECT_TRUE(dict.Remove(key, NULL));
    EXPECT_TRUE(deletion_flag);
    EXPECT_FALSE(dict.HasKey(key));
  }
}

TEST(ValuesTest, DictionaryWithoutPathExpansion) {
  DictionaryValue dict;
  dict.Set("this.is.expanded", Value::CreateNullValue());
  dict.SetWithoutPathExpansion("this.isnt.expanded", Value::CreateNullValue());

  EXPECT_FALSE(dict.HasKey("this.is.expanded"));
  EXPECT_TRUE(dict.HasKey("this"));
  Value* value1;
  EXPECT_TRUE(dict.Get("this", &value1));
  DictionaryValue* value2;
  ASSERT_TRUE(dict.GetDictionaryWithoutPathExpansion("this", &value2));
  EXPECT_EQ(value1, value2);
  EXPECT_EQ(1U, value2->size());

  EXPECT_TRUE(dict.HasKey("this.isnt.expanded"));
  Value* value3;
  EXPECT_FALSE(dict.Get("this.isnt.expanded", &value3));
  Value* value4;
  ASSERT_TRUE(dict.GetWithoutPathExpansion("this.isnt.expanded", &value4));
  EXPECT_EQ(Value::TYPE_NULL, value4->GetType());
}

TEST(ValuesTest, DeepCopy) {
  DictionaryValue original_dict;
  Value* original_null = Value::CreateNullValue();
  original_dict.Set("null", original_null);
  FundamentalValue* original_bool = Value::CreateBooleanValue(true);
  original_dict.Set("bool", original_bool);
  FundamentalValue* original_int = Value::CreateIntegerValue(42);
  original_dict.Set("int", original_int);
  FundamentalValue* original_double = Value::CreateDoubleValue(3.14);
  original_dict.Set("double", original_double);
  StringValue* original_string = Value::CreateStringValue("hello");
  original_dict.Set("string", original_string);
  StringValue* original_string16 =
      Value::CreateStringValue(ASCIIToUTF16("hello16"));
  original_dict.Set("string16", original_string16);

  char* original_buffer = new char[42];
  memset(original_buffer, '!', 42);
  BinaryValue* original_binary = BinaryValue::Create(original_buffer, 42);
  original_dict.Set("binary", original_binary);

  ListValue* original_list = new ListValue();
  FundamentalValue* original_list_element_0 = Value::CreateIntegerValue(0);
  original_list->Append(original_list_element_0);
  FundamentalValue* original_list_element_1 = Value::CreateIntegerValue(1);
  original_list->Append(original_list_element_1);
  original_dict.Set("list", original_list);

  scoped_ptr<DictionaryValue> copy_dict(original_dict.DeepCopy());
  ASSERT_TRUE(copy_dict.get());
  ASSERT_NE(copy_dict.get(), &original_dict);

  Value* copy_null = NULL;
  ASSERT_TRUE(copy_dict->Get("null", &copy_null));
  ASSERT_TRUE(copy_null);
  ASSERT_NE(copy_null, original_null);
  ASSERT_TRUE(copy_null->IsType(Value::TYPE_NULL));

  Value* copy_bool = NULL;
  ASSERT_TRUE(copy_dict->Get("bool", &copy_bool));
  ASSERT_TRUE(copy_bool);
  ASSERT_NE(copy_bool, original_bool);
  ASSERT_TRUE(copy_bool->IsType(Value::TYPE_BOOLEAN));
  bool copy_bool_value = false;
  ASSERT_TRUE(copy_bool->GetAsBoolean(&copy_bool_value));
  ASSERT_TRUE(copy_bool_value);

  Value* copy_int = NULL;
  ASSERT_TRUE(copy_dict->Get("int", &copy_int));
  ASSERT_TRUE(copy_int);
  ASSERT_NE(copy_int, original_int);
  ASSERT_TRUE(copy_int->IsType(Value::TYPE_INTEGER));
  int copy_int_value = 0;
  ASSERT_TRUE(copy_int->GetAsInteger(&copy_int_value));
  ASSERT_EQ(42, copy_int_value);

  Value* copy_double = NULL;
  ASSERT_TRUE(copy_dict->Get("double", &copy_double));
  ASSERT_TRUE(copy_double);
  ASSERT_NE(copy_double, original_double);
  ASSERT_TRUE(copy_double->IsType(Value::TYPE_DOUBLE));
  double copy_double_value = 0;
  ASSERT_TRUE(copy_double->GetAsDouble(&copy_double_value));
  ASSERT_EQ(3.14, copy_double_value);

  Value* copy_string = NULL;
  ASSERT_TRUE(copy_dict->Get("string", &copy_string));
  ASSERT_TRUE(copy_string);
  ASSERT_NE(copy_string, original_string);
  ASSERT_TRUE(copy_string->IsType(Value::TYPE_STRING));
  std::string copy_string_value;
  string16 copy_string16_value;
  ASSERT_TRUE(copy_string->GetAsString(&copy_string_value));
  ASSERT_TRUE(copy_string->GetAsString(&copy_string16_value));
  ASSERT_EQ(std::string("hello"), copy_string_value);
  ASSERT_EQ(ASCIIToUTF16("hello"), copy_string16_value);

  Value* copy_string16 = NULL;
  ASSERT_TRUE(copy_dict->Get("string16", &copy_string16));
  ASSERT_TRUE(copy_string16);
  ASSERT_NE(copy_string16, original_string16);
  ASSERT_TRUE(copy_string16->IsType(Value::TYPE_STRING));
  ASSERT_TRUE(copy_string16->GetAsString(&copy_string_value));
  ASSERT_TRUE(copy_string16->GetAsString(&copy_string16_value));
  ASSERT_EQ(std::string("hello16"), copy_string_value);
  ASSERT_EQ(ASCIIToUTF16("hello16"), copy_string16_value);

  Value* copy_binary = NULL;
  ASSERT_TRUE(copy_dict->Get("binary", &copy_binary));
  ASSERT_TRUE(copy_binary);
  ASSERT_NE(copy_binary, original_binary);
  BinaryValue* binary_value = copy_binary->AsBinary();
  ASSERT_TRUE(binary_value);
  ASSERT_NE(original_binary->GetBuffer(), binary_value->GetBuffer());
  ASSERT_EQ(original_binary->GetSize(), binary_value->GetSize());
  ASSERT_EQ(0, memcmp(original_binary->GetBuffer(),
                      binary_value->GetBuffer(),
                      original_binary->GetSize()));

  Value* copy_value = NULL;
  ASSERT_TRUE(copy_dict->Get("list", &copy_value));
  ASSERT_TRUE(copy_value);
  ASSERT_NE(copy_value, original_list);
  ListValue* copy_list = copy_value->AsList();
  ASSERT_TRUE(copy_list != NULL);
  ASSERT_EQ(2U, copy_list->GetSize());

  Value* copy_list_element_0;
  ASSERT_TRUE(copy_list->Get(0, &copy_list_element_0));
  ASSERT_TRUE(copy_list_element_0);
  ASSERT_NE(copy_list_element_0, original_list_element_0);
  int copy_list_element_0_value;
  ASSERT_TRUE(copy_list_element_0->GetAsInteger(&copy_list_element_0_value));
  ASSERT_EQ(0, copy_list_element_0_value);

  Value* copy_list_element_1;
  ASSERT_TRUE(copy_list->Get(1, &copy_list_element_1));
  ASSERT_TRUE(copy_list_element_1);
  ASSERT_NE(copy_list_element_1, original_list_element_1);
  int copy_list_element_1_value;
  ASSERT_TRUE(copy_list_element_1->GetAsInteger(&copy_list_element_1_value));
  ASSERT_EQ(1, copy_list_element_1_value);
}

TEST(ValuesTest, Equals) {
  Value* null1 = Value::CreateNullValue();
  Value* null2 = Value::CreateNullValue();
  EXPECT_NE(null1, null2);
  EXPECT_TRUE(null1->Equals(null2));

  Value* boolean = Value::CreateBooleanValue(false);
  EXPECT_FALSE(null1->Equals(boolean));
  delete null1;
  delete null2;
  delete boolean;

  DictionaryValue dv;
  dv.SetBoolean("a", false);
  dv.SetInteger("b", 2);
  dv.SetDouble("c", 2.5);
  dv.SetString("d1", "string");
  dv.SetString("d2", ASCIIToUTF16("http://google.com"));
  dv.Set("e", Value::CreateNullValue());

  scoped_ptr<DictionaryValue> copy;
  copy.reset(dv.DeepCopy());
  EXPECT_TRUE(dv.Equals(copy.get()));

  ListValue* list = new ListValue;
  list->Append(Value::CreateNullValue());
  list->Append(new DictionaryValue);
  dv.Set("f", list);

  EXPECT_FALSE(dv.Equals(copy.get()));
  copy->Set("f", list->DeepCopy());
  EXPECT_TRUE(dv.Equals(copy.get()));

  list->Append(Value::CreateBooleanValue(true));
  EXPECT_FALSE(dv.Equals(copy.get()));

  // Check if Equals detects differences in only the keys.
  copy.reset(dv.DeepCopy());
  EXPECT_TRUE(dv.Equals(copy.get()));
  copy->Remove("a", NULL);
  copy->SetBoolean("aa", false);
  EXPECT_FALSE(dv.Equals(copy.get()));
}

TEST(ValuesTest, StaticEquals) {
  scoped_ptr<Value> null1(Value::CreateNullValue());
  scoped_ptr<Value> null2(Value::CreateNullValue());
  EXPECT_TRUE(Value::Equals(null1.get(), null2.get()));
  EXPECT_TRUE(Value::Equals(NULL, NULL));

  scoped_ptr<Value> i42(Value::CreateIntegerValue(42));
  scoped_ptr<Value> j42(Value::CreateIntegerValue(42));
  scoped_ptr<Value> i17(Value::CreateIntegerValue(17));
  EXPECT_TRUE(Value::Equals(i42.get(), i42.get()));
  EXPECT_TRUE(Value::Equals(j42.get(), i42.get()));
  EXPECT_TRUE(Value::Equals(i42.get(), j42.get()));
  EXPECT_FALSE(Value::Equals(i42.get(), i17.get()));
  EXPECT_FALSE(Value::Equals(i42.get(), NULL));
  EXPECT_FALSE(Value::Equals(NULL, i42.get()));

  // NULL and Value::CreateNullValue() are intentionally different: We need
  // support for NULL as a return value for "undefined" without caring for
  // ownership of the pointer.
  EXPECT_FALSE(Value::Equals(null1.get(), NULL));
  EXPECT_FALSE(Value::Equals(NULL, null1.get()));
}

TEST(ValuesTest, DeepCopyCovariantReturnTypes) {
  DictionaryValue original_dict;
  Value* original_null = Value::CreateNullValue();
  original_dict.Set("null", original_null);
  FundamentalValue* original_bool = Value::CreateBooleanValue(true);
  original_dict.Set("bool", original_bool);
  FundamentalValue* original_int = Value::CreateIntegerValue(42);
  original_dict.Set("int", original_int);
  FundamentalValue* original_double = Value::CreateDoubleValue(3.14);
  original_dict.Set("double", original_double);
  StringValue* original_string = Value::CreateStringValue("hello");
  original_dict.Set("string", original_string);
  StringValue* original_string16 =
      Value::CreateStringValue(ASCIIToUTF16("hello16"));
  original_dict.Set("string16", original_string16);

  char* original_buffer = new char[42];
  memset(original_buffer, '!', 42);
  BinaryValue* original_binary = BinaryValue::Create(original_buffer, 42);
  original_dict.Set("binary", original_binary);

  ListValue* original_list = new ListValue();
  FundamentalValue* original_list_element_0 = Value::CreateIntegerValue(0);
  original_list->Append(original_list_element_0);
  FundamentalValue* original_list_element_1 = Value::CreateIntegerValue(1);
  original_list->Append(original_list_element_1);
  original_dict.Set("list", original_list);

  Value* original_dict_value = &original_dict;
  Value* original_bool_value = original_bool;
  Value* original_int_value = original_int;
  Value* original_double_value = original_double;
  Value* original_string_value = original_string;
  Value* original_string16_value = original_string16;
  Value* original_binary_value = original_binary;
  Value* original_list_value = original_list;

  scoped_ptr<Value> copy_dict_value(original_dict_value->DeepCopy());
  scoped_ptr<Value> copy_bool_value(original_bool_value->DeepCopy());
  scoped_ptr<Value> copy_int_value(original_int_value->DeepCopy());
  scoped_ptr<Value> copy_double_value(original_double_value->DeepCopy());
  scoped_ptr<Value> copy_string_value(original_string_value->DeepCopy());
  scoped_ptr<Value> copy_string16_value(original_string16_value->DeepCopy());
  scoped_ptr<Value> copy_binary_value(original_binary_value->DeepCopy());
  scoped_ptr<Value> copy_list_value(original_list_value->DeepCopy());

  EXPECT_TRUE(original_dict_value->Equals(copy_dict_value.get()));
  EXPECT_TRUE(original_bool_value->Equals(copy_bool_value.get()));
  EXPECT_TRUE(original_int_value->Equals(copy_int_value.get()));
  EXPECT_TRUE(original_double_value->Equals(copy_double_value.get()));
  EXPECT_TRUE(original_string_value->Equals(copy_string_value.get()));
  EXPECT_TRUE(original_string16_value->Equals(copy_string16_value.get()));
  EXPECT_TRUE(original_binary_value->Equals(copy_binary_value.get()));
  EXPECT_TRUE(original_list_value->Equals(copy_list_value.get()));
}

TEST(ValuesTest, RemoveEmptyChildren) {
  scoped_ptr<DictionaryValue> root(new DictionaryValue);
  // Remove empty lists and dictionaries.
  root->Set("empty_dict", new DictionaryValue);
  root->Set("empty_list", new ListValue);
  root->SetWithoutPathExpansion("a.b.c.d.e", new DictionaryValue);
  root.reset(root->DeepCopyWithoutEmptyChildren());
  EXPECT_TRUE(root->empty());

  // Make sure we don't prune too much.
  root->SetBoolean("bool", true);
  root->Set("empty_dict", new DictionaryValue);
  root->SetString("empty_string", "");
  root.reset(root->DeepCopyWithoutEmptyChildren());
  EXPECT_EQ(2U, root->size());

  // Should do nothing.
  root.reset(root->DeepCopyWithoutEmptyChildren());
  EXPECT_EQ(2U, root->size());

  // Nested test cases.  These should all reduce back to the bool and string
  // set above.
  {
    root->Set("a.b.c.d.e", new DictionaryValue);
    root.reset(root->DeepCopyWithoutEmptyChildren());
    EXPECT_EQ(2U, root->size());
  }
  {
    DictionaryValue* inner = new DictionaryValue;
    root->Set("dict_with_emtpy_children", inner);
    inner->Set("empty_dict", new DictionaryValue);
    inner->Set("empty_list", new ListValue);
    root.reset(root->DeepCopyWithoutEmptyChildren());
    EXPECT_EQ(2U, root->size());
  }
  {
    ListValue* inner = new ListValue;
    root->Set("list_with_empty_children", inner);
    inner->Append(new DictionaryValue);
    inner->Append(new ListValue);
    root.reset(root->DeepCopyWithoutEmptyChildren());
    EXPECT_EQ(2U, root->size());
  }

  // Nested with siblings.
  {
    ListValue* inner = new ListValue;
    root->Set("list_with_empty_children", inner);
    inner->Append(new DictionaryValue);
    inner->Append(new ListValue);
    DictionaryValue* inner2 = new DictionaryValue;
    root->Set("dict_with_empty_children", inner2);
    inner2->Set("empty_dict", new DictionaryValue);
    inner2->Set("empty_list", new ListValue);
    root.reset(root->DeepCopyWithoutEmptyChildren());
    EXPECT_EQ(2U, root->size());
  }

  // Make sure nested values don't get pruned.
  {
    ListValue* inner = new ListValue;
    root->Set("list_with_empty_children", inner);
    ListValue* inner2 = new ListValue;
    inner->Append(new DictionaryValue);
    inner->Append(inner2);
    inner2->Append(Value::CreateStringValue("hello"));
    root.reset(root->DeepCopyWithoutEmptyChildren());
    EXPECT_EQ(3U, root->size());
    EXPECT_TRUE(root->GetList("list_with_empty_children", &inner));
    EXPECT_EQ(1U, inner->GetSize());  // Dictionary was pruned.
    EXPECT_TRUE(inner->GetList(0, &inner2));
    EXPECT_EQ(1U, inner2->GetSize());
  }
}

TEST(ValuesTest, MergeDictionary) {
  scoped_ptr<DictionaryValue> base(new DictionaryValue);
  base->SetString("base_key", "base_key_value_base");
  base->SetString("collide_key", "collide_key_value_base");
  DictionaryValue* base_sub_dict = new DictionaryValue;
  base_sub_dict->SetString("sub_base_key", "sub_base_key_value_base");
  base_sub_dict->SetString("sub_collide_key", "sub_collide_key_value_base");
  base->Set("sub_dict_key", base_sub_dict);

  scoped_ptr<DictionaryValue> merge(new DictionaryValue);
  merge->SetString("merge_key", "merge_key_value_merge");
  merge->SetString("collide_key", "collide_key_value_merge");
  DictionaryValue* merge_sub_dict = new DictionaryValue;
  merge_sub_dict->SetString("sub_merge_key", "sub_merge_key_value_merge");
  merge_sub_dict->SetString("sub_collide_key", "sub_collide_key_value_merge");
  merge->Set("sub_dict_key", merge_sub_dict);

  base->MergeDictionary(merge.get());

  EXPECT_EQ(4U, base->size());
  std::string base_key_value;
  EXPECT_TRUE(base->GetString("base_key", &base_key_value));
  EXPECT_EQ("base_key_value_base", base_key_value); // Base value preserved.
  std::string collide_key_value;
  EXPECT_TRUE(base->GetString("collide_key", &collide_key_value));
  EXPECT_EQ("collide_key_value_merge", collide_key_value); // Replaced.
  std::string merge_key_value;
  EXPECT_TRUE(base->GetString("merge_key", &merge_key_value));
  EXPECT_EQ("merge_key_value_merge", merge_key_value); // Merged in.

  DictionaryValue* res_sub_dict;
  EXPECT_TRUE(base->GetDictionary("sub_dict_key", &res_sub_dict));
  EXPECT_EQ(3U, res_sub_dict->size());
  std::string sub_base_key_value;
  EXPECT_TRUE(res_sub_dict->GetString("sub_base_key", &sub_base_key_value));
  EXPECT_EQ("sub_base_key_value_base", sub_base_key_value); // Preserved.
  std::string sub_collide_key_value;
  EXPECT_TRUE(res_sub_dict->GetString("sub_collide_key",
                                      &sub_collide_key_value));
  EXPECT_EQ("sub_collide_key_value_merge", sub_collide_key_value); // Replaced.
  std::string sub_merge_key_value;
  EXPECT_TRUE(res_sub_dict->GetString("sub_merge_key", &sub_merge_key_value));
  EXPECT_EQ("sub_merge_key_value_merge", sub_merge_key_value); // Merged in.
}

}  // namespace base
