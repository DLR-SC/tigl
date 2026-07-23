/*
* Copyright (C) 2026 German Aerospace Center (DLR/SC)
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
#ifndef DOCUMENTID_H
#define DOCUMENTID_H

// Identifies one open document (a CPACS configuration or an imported geometry
// file) in TIGLCreator, independent of whether it has a TIGLCreatorDocument
// instance. Used to scope shapes displayed in the shared 3D scene to the
// document that owns them.
using DocumentId = int;

constexpr DocumentId InvalidDocumentId = -1;

#endif // DOCUMENTID_H
