/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-08 Martin Siggel Martin.Siggel@dlr.de>
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

#ifndef TIGLVIEWERCONTROLFILE_H_
#define TIGLVIEWERCONTROLFILE_H_

enum CF_ReturnCode{
	CF_SUCCESS,
	CF_FILE_NOT_FOUND,
	CF_ERROR
};

enum CF_BOOL_STATUS{
	CF_TRUE,
	CF_FALSE,
	UNDEFINED
};

class TIGLViewerControlFile {
public:
	TIGLViewerControlFile();

	CF_ReturnCode read(const char *);

	virtual ~TIGLViewerControlFile();

	CF_BOOL_STATUS showConsole;
};

#endif /* TIGLVIEWERCONTROLFILE_H_ */
