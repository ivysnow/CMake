/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmLocalBorlandMakefileGenerator.h"
#include "cmMakefile.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmMakeDepend.h"
#include "cmCacheManager.h"
#include "cmGeneratedFileStream.h"
#include "windows.h"
#include <stdio.h>

cmLocalBorlandMakefileGenerator::cmLocalBorlandMakefileGenerator()
{
  this->SetLibraryPathOption("-L");
  this->SetLibraryLinkOption("");
}

cmLocalBorlandMakefileGenerator::~cmLocalBorlandMakefileGenerator()
{
}


void cmLocalBorlandMakefileGenerator::OutputMakeVariables(std::ostream& fout)
{
  fout << "# NMake Makefile generated by cmake\n";
  const char* variables = 
    "# general varibles used in the makefile\n"
    "\n"
    "CMAKE_STANDARD_WINDOWS_LIBRARIES = @CMAKE_STANDARD_WINDOWS_LIBRARIES@\n"
    "CMAKE_C_FLAGS        = @CMAKE_C_FLAGS@ @BUILD_FLAGS@\n"
    "CMAKE_OBJECT_FILE_SUFFIX            = @CMAKE_OBJECT_FILE_SUFFIX@\n"
    "CMAKE_EXECUTABLE_SUFFIX             = @CMAKE_EXECUTABLE_SUFFIX@\n"
    "CMAKE_STATICLIB_SUFFIX              = @CMAKE_STATICLIB_SUFFIX@\n"
    "CMAKE_SHLIB_SUFFIX                  = @CMAKE_SHLIB_SUFFIX@\n" 
    "CMAKE_SHLIB_CFLAGS                  = -tWR\n" 
    "CMAKE_LINKER_FLAGS                  = @CMAKE_LINKER_FLAGS@ @LINKER_BUILD_FLAGS@\n"
    "CMAKE_CXX_FLAGS      = -P @CMAKE_CXX_FLAGS@ @BUILD_FLAGS@\n"
    "!IF \"$(OS)\" == \"Windows_NT\"\n"
    "NULL=\n"
    "!ELSE \n"
    "NULL=nul\n"
    "!ENDIF \n";
  std::string buildType = "CMAKE_CXX_FLAGS_";
  buildType +=  m_Makefile->GetDefinition("CMAKE_BUILD_TYPE");
  buildType = cmSystemTools::UpperCase(buildType);
  m_Makefile->AddDefinition("BUILD_FLAGS",
                            m_Makefile->GetDefinition(
                              buildType.c_str()));
  
  buildType = "CMAKE_LINKER_FLAGS_";
  buildType +=  m_Makefile->GetDefinition("CMAKE_BUILD_TYPE");
  buildType = cmSystemTools::UpperCase(buildType);
  m_Makefile->AddDefinition("LINKER_BUILD_FLAGS",
                            m_Makefile->GetDefinition(
                              buildType.c_str()));

  std::string replaceVars = variables;
  m_Makefile->ExpandVariablesInString(replaceVars);
  std::string ccompiler = m_Makefile->GetDefinition("CMAKE_C_COMPILER");
  fout << "CMAKE_C_COMPILER  = " 
       << cmSystemTools::ConvertToOutputPath(ccompiler.c_str())
       << "\n";
  std::string cxxcompiler = m_Makefile->GetDefinition("CMAKE_CXX_COMPILER");
  fout << "CMAKE_CXX_COMPILER  = "
       << cmSystemTools::ConvertToOutputPath(cxxcompiler.c_str())
       << "\n";
  
  if(m_Makefile->GetDefinition("CMAKE_EDIT_COMMAND"))
    {
    fout << "CMAKE_EDIT_COMMAND = "
         << this->ShortPath(m_Makefile->GetDefinition("CMAKE_EDIT_COMMAND"))
         << "\n";
    }
  
  std::string cmakecommand = m_Makefile->GetDefinition("CMAKE_COMMAND");
  fout << "CMAKE_COMMAND = " 
       << this->ShortPath(cmakecommand.c_str()) << "\n";
  fout << "RM = " << this->ShortPath(cmakecommand.c_str()) << " -E remove -f\n";

  fout << replaceVars.c_str();
  fout << "CMAKE_CURRENT_SOURCE = " 
       << ShortPath(m_Makefile->GetStartDirectory() )
       << "\n";
  fout << "CMAKE_CURRENT_BINARY = " 
       << ShortPath(m_Makefile->GetStartOutputDirectory())
       << "\n";
  fout << "CMAKE_SOURCE_DIR = " 
       << ShortPath(m_Makefile->GetHomeDirectory()) << "\n";
  fout << "CMAKE_BINARY_DIR = " 
       << ShortPath(m_Makefile->GetHomeOutputDirectory() )
       << "\n";
  // Output Include paths
  fout << "INCLUDE_FLAGS = ";
  std::vector<std::string>& includes = m_Makefile->GetIncludeDirectories();
  std::vector<std::string>::iterator i;
  fout << "-I" << 
    this->ShortPath(m_Makefile->GetStartDirectory()) << " ";
  for(i = includes.begin(); i != includes.end(); ++i)
    {
    std::string include = *i;
    // Don't output a -I for the standard include path "/usr/include".
    // This can cause problems with certain standard library
    // implementations because the wrong headers may be found first.
    fout << "-I" << cmSystemTools::ConvertToOutputPath(i->c_str()).c_str() << " ";
    } 
  fout << m_Makefile->GetDefineFlags();
  fout << "\n\n";
}



void 
cmLocalBorlandMakefileGenerator::
OutputBuildObjectFromSource(std::ostream& fout,
                            const char* shortName,
                            const cmSourceFile& source,
                            const char* extraCompileFlags,
                            bool shared)
{
  // force shared flag if building shared libraries
  if(cmSystemTools::IsOn(m_Makefile->GetDefinition("BUILD_SHARED_LIBS")))
    {
    shared = true;
    }
  // Header files shouldn't have build rules.
  if(source.GetPropertyAsBool("HEADER_FILE_ONLY"))
    {
    return;
    }
  


  std::string comment = "Build ";
  std::string objectFile = std::string(shortName) + 
    this->GetOutputExtension(source.GetSourceExtension().c_str());
  objectFile = cmSystemTools::ConvertToOutputPath(objectFile.c_str());
  comment += objectFile + "  From ";
  comment += source.GetFullPath();
  std::string compileCommand;
  std::string ext = source.GetSourceExtension();
  if(ext == "c" )
    {
    compileCommand = "$(CMAKE_C_COMPILER) $(CMAKE_C_FLAGS) ";
    compileCommand += extraCompileFlags;
    if(shared)
      {
      compileCommand += "$(CMAKE_SHLIB_CFLAGS) ";
      }
    compileCommand += " -o";
    compileCommand += objectFile;
    compileCommand += " $(INCLUDE_FLAGS) -c ";
    compileCommand += 
      cmSystemTools::ConvertToOutputPath(source.GetFullPath().c_str());
    }
  else if (ext == "rc")
    {
    compileCommand = "$(RC) $(INCLUDE_FLAGS) -o\"";
    compileCommand += objectFile;
    compileCommand += "\" ";
    compileCommand += 
      cmSystemTools::ConvertToOutputPath(source.GetFullPath().c_str());
    }
  else if (ext == "def")
    {
    // no rule to output for this one
    return;
    }
  // assume c++ if not c rc or def
  else
    {
    compileCommand = "$(CMAKE_CXX_COMPILER) $(CMAKE_CXX_FLAGS) ";
    compileCommand += extraCompileFlags;
    if(shared)
      {
      compileCommand += "$(CMAKE_SHLIB_CFLAGS) ";
      }
    compileCommand += " -o";
    compileCommand += objectFile;
    compileCommand += " $(INCLUDE_FLAGS) -c ";
    compileCommand += 
      cmSystemTools::ConvertToOutputPath(source.GetFullPath().c_str());
    }
  this->OutputMakeRule(fout,
                       comment.c_str(),
                       objectFile.c_str(),
                       cmSystemTools::ConvertToOutputPath(
                         source.GetFullPath().c_str()).c_str(),
                       compileCommand.c_str());
}

void cmLocalBorlandMakefileGenerator::OutputSharedLibraryRule(std::ostream& fout, 
                                                       const char* name,
                                                       const cmTarget &t)
{
  std::string target = m_LibraryOutputPath + name;
  std::string libpath = target + ".lib";
  target += ".dll";
  target = cmSystemTools::ConvertToOutputPath(target.c_str());
  libpath = cmSystemTools::ConvertToOutputPath(libpath.c_str());
  std::string depend = "$(";
  depend += this->CreateMakeVariable(name, "_SRC_OBJS");
  depend += ") $(" + this->CreateMakeVariable(name, "_DEPEND_LIBS") + ")";
  std::string command = "$(CMAKE_CXX_COMPILER) -tWD $(CMAKE_SHLIB_CFLAGS) $(CMAKE_LINKER_FLAGS) @&&|\n";
  // must be executable name
  command += "-e";
  command += target;
  command += " ";
  cmOStringStream linklibs;
  this->OutputLinkLibraries(linklibs, name, t);
  // then the linker options -L and libraries (any other order will fail!)
  command += linklibs.str();
  // then list of object files 
  command += " $(" + this->CreateMakeVariable(name, "_SRC_OBJS") + ") ";
  std::string command2 = "implib -w ";
  command2 += libpath + " " + target;
  const std::vector<cmSourceFile*>& sources = t.GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = sources.begin();
      i != sources.end(); ++i)
    {
    if((*i)->GetSourceExtension() == "def")
      {
      command += "";
      command += (*i)->GetFullPath();
      }
    }
  command += "\n|\n";
    
  std::string customCommands = this->CreateTargetRules(t, name);
  const char* cc = 0;
  if(customCommands.size() > 0)
    {
    cc = customCommands.c_str();
    }
  this->OutputMakeRule(fout, "rules for a shared library",
                       target.c_str(),
                       depend.c_str(),
                       command.c_str(),
                       command2.c_str(),
                       cc);
}

void cmLocalBorlandMakefileGenerator::OutputModuleLibraryRule(std::ostream& fout, 
                                                       const char* name, 
                                                       const cmTarget &target)
{
  this->OutputSharedLibraryRule(fout, name, target);
}

void cmLocalBorlandMakefileGenerator::OutputStaticLibraryRule(std::ostream& fout, 
                                                       const char* name,
                                                       const cmTarget &t)
{
  std::string target = m_LibraryOutputPath + std::string(name) + ".lib";
  target = cmSystemTools::ConvertToOutputPath(target.c_str());
  std::string depend = "$(";
  depend += this->CreateMakeVariable(name, "_SRC_OBJS") + ") ";
  std::string command = "tlib  @&&|\n\t /p512 /a ";
  command += target;
  command += " ";
  std::string deleteCommand = "if exist ";
  deleteCommand +=  target;
  deleteCommand += " del ";
  deleteCommand += target;

  command += " $(";
  command += this->CreateMakeVariable(name, "_SRC_OBJS_QUOTED") + ")";
  command += "\n|";
  std::string comment = "rule to build static library: ";
  comment += name;
  std::string customCommands = this->CreateTargetRules(t, name);
  const char* cc = 0;
  if(customCommands.size() > 0)
    {
    cc = customCommands.c_str();
    }
  this->OutputMakeRule(fout,
                       comment.c_str(),
                       target.c_str(),
                       depend.c_str(),
                       deleteCommand.c_str(),
                       command.c_str(), cc);
}

void cmLocalBorlandMakefileGenerator::OutputExecutableRule(std::ostream& fout,
                                                    const char* name,
                                                    const cmTarget &t)
{
  std::string target = m_ExecutableOutputPath + name + m_ExecutableExtension;
  target = cmSystemTools::ConvertToOutputPath(target.c_str());
  std::string depend = "$(";
  depend += this->CreateMakeVariable(name, "_SRC_OBJS") + ") $(" + 
    this->CreateMakeVariable(name, "_DEPEND_LIBS") + ")";
  std::string command = 
    "$(CMAKE_CXX_COMPILER) @&&|\n";
  command += " $(CMAKE_LINKER_FLAGS) -e" + target;
  if(cmSystemTools::IsOn(m_Makefile->GetDefinition("BUILD_SHARED_LIBS")))
    {
    command += " $(CMAKE_SHLIB_CFLAGS) ";
    }
  if(t.GetType() == cmTarget::WIN32_EXECUTABLE)
    {
    command +=  "-tW -tWM ";
    }
  else
    {
    command += " -tWC ";
    }
  cmOStringStream linklibs;
  this->OutputLinkLibraries(linklibs, 0, t);
  command += linklibs.str();
  command += " $(" +  this->CreateMakeVariable(name, "_SRC_OBJS") + ")";
  command += "\n|";
  std::string comment = "rule to build executable: ";
  comment += name;
  std::string customCommands = this->CreateTargetRules(t, name);
  const char* cc = 0;
  if(customCommands.size() > 0)
    {
    cc = customCommands.c_str();
    }
  this->OutputMakeRule(fout, 
                       comment.c_str(),
                       target.c_str(),
                       depend.c_str(),
                       command.c_str(), cc);
}

  


std::string cmLocalBorlandMakefileGenerator::GetOutputExtension(const char* s)
{
  std::string sourceExtension = s;
  if(sourceExtension == "def")
    {
    return "";
    }
  if(sourceExtension == "ico" || sourceExtension == "rc2")
    {
    return "";
    }
  if(sourceExtension == "rc")
    {
    return ".res";
    }
  return ".obj";
}



bool cmLocalBorlandMakefileGenerator::SamePath(const char* path1, const char* path2)
{
  // first check to see if they are the same anyway
  if (strcmp(path1, path2) == 0)
    {
    return true;
    }
  // next short path and lower case both of them for the compare
  return 
    cmSystemTools::LowerCase(ShortPath(path1)) ==
    cmSystemTools::LowerCase(ShortPath(path2));
}


// borland make does not support variables that are longer than 32
// so use this function to rename any long ones
std::string cmLocalBorlandMakefileGenerator::CreateMakeVariable(const char* s, const char* s2)
{
  std::string unmodified = s;
  unmodified += s2;
  // see if th
  std::map<cmStdString, cmStdString>::iterator i = m_MakeVariableMap.find(unmodified);
  if(i != m_MakeVariableMap.end())
    {
    return i->second;
    }
  std::string ret = unmodified;
  // if the string is greater the 32 chars it is an invalid vairable name
  // for borland make
  if(ret.size() > 32)
    {
    std::string str1 = s;
    std::string str2 = s2;
    // we must shorten the combined string by 4 charactors
    // keep no more than 24 charactors from the second string
    if(str2.size() > 24)
      {
      str2 = str2.substr(0, 24);
      }
    if(str1.size() + str2.size() > 27)
      {
      str1 = str1.substr(0, 27 - str2.size());
      }
    char buffer[5];
    int i = 0;
    sprintf(buffer, "%04d", i);
    ret = str1 + str2 + buffer;
    while(m_ShortMakeVariableMap.count(ret) && i < 1000)
      {
      ++i;
      sprintf(buffer, "%04d", i);
      ret = str1 + str2 + buffer;
      }
    if(i == 1000)
      {
      cmSystemTools::Error("Borland makefile varible length too long");
      return unmodified;
      }
    // once an unused variable is found 
    m_ShortMakeVariableMap[ret] = "1";
    }
  // always make an entry into the unmodified to varible map
  m_MakeVariableMap[unmodified] = ret;
  return ret;
}

