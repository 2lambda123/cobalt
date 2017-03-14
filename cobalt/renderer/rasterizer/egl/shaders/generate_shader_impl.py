#!/usr/bin/python
# Copyright 2016 Google Inc. All Rights Reserved.
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

"""Parse GLSL shader files to generate C++ classes.

This script parses all GLSL shader input files to generate C++ class
definitions which inherit from ShaderBase:

  cobalt/renderer/rasterizer/egl/shader_base.h

The classes will have member functions to access any "attribute" or "uniform"
member(s) found in the shader file. For example, a shader file with the
following lines:

  uniform mat3 u_view_matrix;
  uniform sampler2d intexture;
  attribute vec2 inposition;
  attribute float a_depth;

will have the following member functions in the generated class:

  GLuint intexture() const;
  GLenum intexture_texunit() const;
  GLuint u_view_matrix() const;
  GLuint inposition() const;
  GLuint a_depth() const;

Additionally, the generated classes will include definitions to bind attribute
locations, query uniform locations, and initialize uniform samplers to default
texture units. These are done in the class' InitializePreLink,
InitializePostLink, and InitializePostUse functions, respectively.
"""

import datetime
import os
import re
import sys


def GetBasename(filename):
  """Returns the filename without the directory and without its extension.

  Args:
    filename: A filename from which a basename is to be extracted.

  Returns:
    A string derived from the filename except the directory and extension
    are removed.
  """
  return os.path.splitext(os.path.basename(filename))[0]


def GetShaderClassName(filename):
  """Returns a C++ class name based on the given file name."""
  # Convert underscore-delineated string to camelcase and prepends 'Shader'.
  class_name = ''.join(x.capitalize() for x in
                       GetBasename(filename).lower().split('_'))
  return 'Shader' + class_name


def GetDataDefinitionStringForFile(filename):
  """Returns a string containing C++ array definition for file contents."""
  with open(filename, 'rb') as f:
    # Read the file contents; remove carriage return (apitrace doesn't handle
    # shader sources with that character very well); and add a null terminator
    # at the end.
    file_contents = f.read().replace('\r', '') + '\0'
    def GetChunk(contents, chunk_size):
      # Yield successive |chunk_size|-sized chunks from |contents|.
      for i in xrange(0, len(contents), chunk_size):
        yield contents[i:i + chunk_size]

    # Break up the data into chunk sizes such that the produced output lines
    # representing the data in the .h file are less than 80 characters long.
    length_of_output_byte_string = 6
    max_characters_per_line = 80
    chunk_size = max_characters_per_line / length_of_output_byte_string

    # Convert each byte to ASCII hexadecimal form and output that to the C++
    # header file, line-by-line.
    data_definition_string = '{\n'
    for output_line_data in GetChunk(file_contents, chunk_size):
      data_definition_string += (
          '  ' +
          ' '.join(['0x%02x,' % ord(y) for y in output_line_data]) +
          '\n')
    data_definition_string += '};\n'
    return data_definition_string


def GetShaderSourceDefinitions(files):
  """Returns a string containing C++ that represents all data in files."""
  source_definition_string = ''
  for filename in files:
    class_name = GetShaderClassName(filename)
    source_definition_string += '\nconst char %s::kSource[] = ' % class_name
    source_definition_string += GetDataDefinitionStringForFile(filename)
  return source_definition_string


SOURCE_FILE_TEMPLATE = """\
// Copyright {year} Google Inc.  All Rights Reserved.
// This file is generated by:
//   cobalt/renderer/rasterizer/egl/generate_shader_impl.py
// Do not edit!

#include "{header_filename}"

namespace cobalt {{
namespace renderer {{
namespace rasterizer {{
namespace egl {{
{source_definitions}
}}  // namespace egl
}}  // namespace rasterizer
}}  // namespace renderer
}}  // namespace cobalt
"""


def GenerateSourceFile(source_filename, header_filename, all_shaders):
  """Generate the actual C++ source file."""
  header_filename = os.path.basename(header_filename)
  current_year = datetime.datetime.now().year
  with open(source_filename, 'w') as output_file:
    output_file.write(
        SOURCE_FILE_TEMPLATE.format(
            year=current_year,
            header_filename=header_filename,
            source_definitions=GetShaderSourceDefinitions(all_shaders)))


def IndentLines(lines, indent_count):
  """Indent all lines by the given amount."""
  line_start = '\n' + ' ' * indent_count
  return lines.replace('\n', line_start)


def GetShaderInputs(filename):
  """Return all attributes, uniforms, and samplers in the given shader file."""
  attributes = []
  uniforms = []
  samplers = []

  with open(filename, 'r') as f:
    # Read file as a single string to facilitate comment removal.
    file_contents = f.read()

    # Remove comments and directives.
    file_contents = re.sub(r'/\*.*?\*/', '', file_contents, flags=re.DOTALL)
    file_contents = re.sub(r'\#.*', '', file_contents)
    file_contents = re.sub(r'//.*', '', file_contents)

    # Remove everything associated with the main program.
    file_contents = re.sub(r'void main\(\) .*', '', file_contents,
                           flags=re.DOTALL)

    # Match attributes, uniforms, and samplers (a subset of uniforms).
    for line in file_contents.split(';'):
      words = line.strip().split()
      if len(words) == 3:
        type_name = words[0].lower()
        if type_name == 'attribute':
          attributes.append(words[2])
        elif type_name == 'uniform':
          uniforms.append(words[2])
          if words[1].lower().startswith('sampler'):
            samplers.append(words[2])

  return attributes, uniforms, samplers


def GetAttributeMethods(attributes):
  """Return a string representing C++ methods for the given attributes."""
  methods = ''
  for index, name in enumerate(attributes):
    methods += '\nGLuint {0}() const {{ return {1}; }}'.format(name, index)
  return methods


def GetUniformMethods(uniforms):
  """Return a string representing C++ methods for the given uniforms."""
  methods = ''
  for name in uniforms:
    methods += '\nGLuint {0}() const {{ return {0}_; }}'.format(name)
  return methods


def GetSamplerMethods(samplers):
  """Returns a string representing C++ methods for the given samplers."""
  methods = ''
  for index, name in enumerate(samplers):
    methods += ('\nGLenum {0}_texunit() const {{ return GL_TEXTURE{1}; }}'
                .format(name, index))
  return methods


def GetInitializePreLink(attributes):
  """Returns a string representing C++ statements to process during prelink."""
  statements = ''
  for name in attributes:
    statements += '\nBindAttribLocation(program, {0}(), "{0}");'.format(name)
  return statements


def GetInitializePostLink(uniforms):
  """Returns a string representing C++ statements to process during postlink."""
  statements = ''
  for name in uniforms:
    statements += '\n{0}_ = GetUniformLocation(program, "{0}");'.format(name)
  return statements


def GetInitializePostUse(samplers):
  """Returns a string representing C++ statements to process during postuse."""
  statements = ''
  for name in samplers:
    statements += ('\nSetTextureUnitForUniformSampler({0}(), {0}_texunit());'
                   .format(name))
  return statements


def GetVariables(variable_names):
  """Returns a string representing C++ member variable declarations."""
  variables = ''
  for name in variable_names:
    variables += '\nGLuint {0}_;'.format(name)
  return variables


CLASS_TEMPLATE = """\
class {class_name} : public ShaderBase {{
 public:
  const char* GetSource() const OVERRIDE {{ return kSource; }}
{attribute_methods}\
{uniform_methods}\
{sampler_methods}

 private:
  void InitializePreLink(GLuint program) OVERRIDE {{\
{initialize_prelink}
  }}
  void InitializePostLink(GLuint program) OVERRIDE {{\
{initialize_postlink}
  }}
  void InitializePostUse() OVERRIDE {{\
{initialize_postuse}
  }}
{variables}

  static const char kSource[];
}};
"""


def GetClassDefinitionForFile(filename):
  """Returns a string representing a C++ class definition for a shader."""
  attributes, uniforms, samplers = GetShaderInputs(filename)
  return CLASS_TEMPLATE.format(
      class_name=GetShaderClassName(filename),
      # For the following fields, each line in the strings should start with
      # a newline.
      attribute_methods=IndentLines(GetAttributeMethods(attributes), 2),
      uniform_methods=IndentLines(GetUniformMethods(uniforms), 2),
      sampler_methods=IndentLines(GetSamplerMethods(samplers), 2),
      initialize_prelink=IndentLines(GetInitializePreLink(attributes), 4),
      initialize_postlink=IndentLines(GetInitializePostLink(uniforms), 4),
      initialize_postuse=IndentLines(GetInitializePostUse(samplers), 4),
      variables=IndentLines(GetVariables(uniforms), 2))


def GetShaderClassDefinitions(files):
  """Returns a string representing C++ class definitions for all given files."""
  class_definition_string = ''
  for filename in files:
    class_definition_string += '\n' + GetClassDefinitionForFile(filename)
  return class_definition_string


HEADER_FILE_TEMPLATE = """\
// Copyright {year} Google Inc.  All Rights Reserved.
// This file is generated by:
//   cobalt/renderer/rasterizer/egl/generate_shader_impl.py
// Do not edit!

#ifndef {include_guard}
#define {include_guard}

#include "base/compiler_specific.h"
#include "cobalt/renderer/rasterizer/egl/shader_base.h"

namespace cobalt {{
namespace renderer {{
namespace rasterizer {{
namespace egl {{
{class_definitions}
}}  // namespace egl
}}  // namespace rasterizer
}}  // namespace renderer
}}  // namespace cobalt

#endif  // {include_guard}
"""


def GenerateHeaderFile(output_filename, all_shaders):
  """Generate the actual C++ header file."""
  include_guard = 'GENERATED_%s_H_' % GetBasename(output_filename).upper()
  current_year = datetime.datetime.now().year
  with open(output_filename, 'w') as output_file:
    output_file.write(
        HEADER_FILE_TEMPLATE.format(
            year=current_year,
            include_guard=include_guard,
            class_definitions=GetShaderClassDefinitions(all_shaders)))


def main(output_header_filename, output_source_filename, shader_files_file):
  all_shader_files = []
  with open(shader_files_file, 'r') as input_file:
    shader_files = input_file.readlines()
    for filename in shader_files:
      all_shader_files.append(filename.strip())

  GenerateSourceFile(output_source_filename, output_header_filename,
                     all_shader_files)
  GenerateHeaderFile(output_header_filename, all_shader_files)


if __name__ == '__main__':
  main(sys.argv[1], sys.argv[2], sys.argv[3])
