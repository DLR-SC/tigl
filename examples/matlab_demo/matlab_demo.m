% Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
%
% Created: 2014-04-14 Martin Siggel <Martin.Siggel@dlr.de>
%
% Licensed under the Apache License, Version 2.0 (the "License");
% you may not use this file except in compliance with the License.
% You may obtain a copy of the License at
%
%     http://www.apache.org/licenses/LICENSE-2.0
%
% Unless required by applicable law or agreed to in writing, software
% distributed under the License is distributed on an "AS IS" BASIS,
% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
% See the License for the specific language governing permissions and
% limitations under the License.


function matlab_demo(file)
    try
        tixiHandle = tixiOpenDocument(file);
    catch err
        error('Error opening cpacs file with TiXI')
    end


    % enable logging of errors and warnings into file
    tiglLogSetFileEnding('txt');
    tiglLogSetTimeInFilenameEnabled(0);
    tiglLogToFileEnabled('demolog');

    % open cpacs with tigl
    try
        tiglHandle = tiglOpenCPACSConfiguration(tixiHandle, '');
    catch err
        error('Error opening cpacs file with TiGL')
    end

    % query number of wings and fuselages and their names
    nWings = tiglGetWingCount(tiglHandle);
    nFuselages = tiglGetFuselageCount(tiglHandle);
    for i = 1:nWings
        wingUID = tiglWingGetUID(tiglHandle, i);
        fprintf(1, 'Wing %d name: %s\n', i, wingUID);
    end

    for i = 1:nFuselages
        fuselageUID = tiglFuselageGetUID(tiglHandle, i);
        fprintf(1, 'Fuselage %d name: %s\n', i, fuselageUID);
    end

    % query point on the upper wing surface
    if nWings > 0 
        [x,y,z] = tiglWingGetUpperPoint(tiglHandle, 1, 1, 0.5, 0.5);
        fprintf(1, 'Point on upper wing surface of wing 1/segment 1: (x,y,z) = (%g, %g, %g)\n', x, y, z);
    end

    % compute intersection of wing with a x-z plane
    if nWings > 0 
        wingUID = tiglWingGetUID(tiglHandle, 1);
        % do the intersection with a plane at p = (0,0.5,0) and normal vector n = (0,1,0)
        int_id = tiglIntersectWithPlane(tiglHandle, wingUID, 0, 0.5, 0, 0, 1, 0);

        % get number of intersection wires
        nlines = tiglIntersectGetLineCount(tiglHandle, int_id);

        % query points on the first line
        if nlines > 0
            zeta = 0.;
            fprintf(1, '\nIntersection points of a plane with first wing:\n');
            while zeta < 1 
                [x,y,z] = tiglIntersectGetPoint(tiglHandle, int_id, 1, zeta);
                fprintf(1, 'zeta = %g\tp=(%g, %g, %g)\n', zeta, x, y, z);
                zeta = zeta + 0.1;
            end
            fprintf(1, '\n');
        end
    end

    % Export geometry to iges file
    fprintf(1, 'Exporting geometry to igesexport.igs\n');
    tiglExportIGES(tiglHandle, 'igesexport.igs');

    tiglLogToFileDisabled();
    tiglCloseCPACSConfiguration(tiglHandle);
    tixiCloseDocument(tixiHandle);
    tixiCleanup();
end