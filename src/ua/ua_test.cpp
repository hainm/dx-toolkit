// Copyright (C) 2016 DNAnexus, Inc.
//
// This file is part of dx-toolkit (DNAnexus platform client libraries).
//
//   Licensed under the Apache License, Version 2.0 (the "License"); you may
//   not use this file except in compliance with the License. You may obtain a
//   copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
//   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
//   License for the specific language governing permissions and limitations
//   under the License.



// This function should be called before opt.setApiserverDxConfig() is called,
// since opt::setApiserverDxConfig() changes the value of dx::config::*, based on command line args

#include <string>
#include "ua_test.h"
#include "dxcpp/dxcpp.h"
#include "api_helper.h"
#include "options.h"
#include "api.h"

#if WINDOWS_BUILD
#include <windows.h>
#else
#include <sys/utsname.h>
#endif

using namespace std;
using namespace dx;
using namespace dx::config;

void runTests()
{
  version();
  printEnvironmentInfo();
  proxySettings();  
  osInfo();
  certificateFile();
  testWhoAmI();
  currentProject();
  contactGoogle();
}

void version() {
  cout << "Upload Agent Version: " << UAVERSION;
#if OLD_KERNEL_SUPPORT
  cout << " (old-kernel-support)";
#endif
  cout << endl
       << "  git version: " << DXTOOLKIT_GITVERSION << endl
       << "  libboost version: " << (BOOST_VERSION / 100000) << "." << ((BOOST_VERSION / 100) % 1000) << "." << (BOOST_VERSION % 100) << endl
       << "  libcurl version: " << LIBCURL_VERSION_MAJOR << "." << LIBCURL_VERSION_MINOR << "." << LIBCURL_VERSION_PATCH << endl;
}

void osInfo(){
#if WINDOWS_BUILD
  OSVERSIONINFO vi;
  vi.dwOSVersionInfoSize = sizeof(vi);
  try {
    GetVersionEx(&vi);
  } catch(Exception &e){
    cout << "Unable to get OS information" << endl;
  }
  cout << "Operating System:" << endl
       << "  Windows: " << vi.dwMajorVersion << "." << dwMinorVersion << "." << dwBuildNumber << "." << dwPlatformId << " " << szCSDVersion << endl;
#else
  struct utsname uts;
  uname(&uts);
  cout << "Operating System:" << endl
       << "  Name:    " << uts.sysname << endl 
       << "  Release: " << uts.release << endl
       << "  Version: " << uts.version << endl
       << "  Machine: " << uts.machine << endl;
#endif
}

void printEnvironmentInfo() {
  cout << "Upload Agent v" << UAVERSION << ", environment info:" << endl
       << "  API server protocol: " << APISERVER_PROTOCOL() << endl
       << "  API server host: " << APISERVER_HOST() << endl
       << "  API server port: " << APISERVER_PORT() << endl;

  if (SECURITY_CONTEXT().size() != 0)
    cout << "  Auth token: " << SECURITY_CONTEXT()["auth_token"].get<string>() << endl;
  else
    cout << "  Auth token: " << endl;

 cout << "  dx toolkit git " << string(DXTOOLKIT_GITVERSION) << endl;
}

void currentProject() {
  string projID = CURRENT_PROJECT();
  try {
    string projName = getProjectName(projID);
    cout << "Current Project: " << projName << " (" << projID << ")" << endl;
  } catch (DXAPIError &e) {
    cout << "  Project: " << projID << endl;
  }
}

bool getProxyValue(const char * name, string &value) {
  if (getenv(name) == NULL)
    return false;
  value = string(getenv(name));
  // Remove credentials from string
  std::size_t atSimbol = value.find_first_of("@");
  if (atSimbol != string::npos ) {
    if (value.substr(0, 7) == "http://") {
      value.replace(7, atSimbol-7, "****");
    } else if (value.substr(0, 8) == "https://") {
      value.replace(8, atSimbol-8, "****");
    } else {
      value.replace(0, atSimbol, "****");
    }
    cout << "  To see actual username and password run: echo $" << name << endl;
    cout << "  Note that special characters in username / password might prevent credentials from being resolved properly." << endl;
  }
  return true;
}


void proxySettings() {
  string value;
  cout << "Proxy Settings:" << endl;
  bool proxySet = false;
  if (getProxyValue("http_proxy", value))  { cout << "  http_proxy: " << value << endl; proxySet = true;}
  if (getProxyValue("https_proxy", value)) { cout << "  https_proxy: " << value << endl;proxySet = true;}
  if (getProxyValue("HTTP_PROXY", value))  { cout << "  HTTP_PROXY: " << value << endl;proxySet = true;}
  if (getProxyValue("HTTPS_PROXY", value)) { cout << "  HTTP_PROXY: " << value << endl;proxySet = true;}
  if (!proxySet) { cout << "  No proxy set in environment." << endl; }
}

void certificateFile() {
  cout << "CA Certificate:" << CA_CERT() << endl;
}

void testWhoAmI(){
  cout << "Testing who am I:" << endl;
  try {
    JSON res = systemWhoami(string("{}"), false);
    cout << "  User: " << res["id"].get<string>() << endl;
  } catch(DXAPIError &e) {
    cout << "Error contacting the api: " << e.what() << endl;
  } catch (DXConnectionError &e) {
    cout << "Error contacting the api: " << e.what() << endl;
  } catch (...) {
    cout << "Error contacting the api." << endl;
  }
}

void contactGoogle() {
  cout << "Testing Contact Google." << endl;
  try {    
    string url = "http://www.google.com/";
    HttpRequest req = HttpRequest::request(dx::HTTP_GET, url);
    if (req.responseCode == 200) {
      cout << "  Sucessfully contacted google.com over http: (" << req.responseCode << ")" << endl;
    } else {
      cout << "  Unable to contact google.com over http: (" << req.responseCode << ")" << endl;
    }
  } catch (HttpRequestException &e) {
    cout << "Error contacting google over http: " << e.what();
  } catch (...) {
    cout << "Error contacting the api." << endl;
  }

  try {
    string url = "https://www.google.com/";
    HttpRequest req = HttpRequest::request(dx::HTTP_GET, url);
    if (req.responseCode == 200) {
      cout << "  Sucessfully contacted google.com over https: (" << req.responseCode << ")" << endl;
    } else {
      cout << "  Unable to contact google.com over https: (" << req.responseCode << ")" << endl;
    }
  } catch (HttpRequestException &e) {
    cout << "Error contacting google over https: " << e.what();
  } catch (...) {
    cout << "Error contacting google" << endl;
  } 

}

