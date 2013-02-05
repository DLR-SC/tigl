/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-05 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef TIGLVIEWERSETTINGS_H_
#define TIGLVIEWERSETTINGS_H_

class TIGLViewerSettings {
public:
	TIGLViewerSettings();
	void loadSettings();
	void storeSettings();

	void setTesselationAccuracy(double);
	void setTriangulationAccuracy(double);

	double tesselationAccuracy() const;
	double triangulationAccuracy() const;

	virtual ~TIGLViewerSettings();

private:
	double _tesselationAccuracy;
	double _triangulationAccuracy;
};

#endif /* TIGLVIEWERSETTINGS_H_ */
