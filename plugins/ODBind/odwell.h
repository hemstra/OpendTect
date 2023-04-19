#pragma once
/*+
________________________________________________________________________________

 Copyright:  (C) 2022 dGB Beheer B.V.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
________________________________________________________________________________

-*/
#include "odbindmod.h"

#include "refcount.h"
#include "welldata.h"
#include "wellman.h"

#include "odbind.h"
#include "odsurvey_object.h"

namespace Well{
    class Track;
}

class ODSurvey;

class odWell : public odSurveyObject
{
public:
    enum SampleMode { Upscale, Sample };
    enum ZMode { MD, TVD, TVDSS, TWT };
    odWell( const odSurvey& thesurvey, const char* name );

    const Well::Data*	wd() const;

    BufferStringSet*	getLogNames() const;
    void		getLogInfo(OD::JSON::Array&,
				   const BufferStringSet&) const;
    BufferStringSet*	getMarkerNames() const;
    void		getMarkerInfo(OD::JSON::Array&,
				      const BufferStringSet&) const;

    void		getTrack(hAllocator);

   //  py::dict    getLogs(py::list lognms, float zstep=0.5,
			// SampleMode samplemode=Upscale);
   //  py::object  getLogsDF(py::list lognms, float zstep=0.5,
			//   SampleMode samplemode=Upscale);

    void		getInfo(OD::JSON::Object&) const override;
    void		getFeature(OD::JSON::Object&,
				   bool towgs=true) const override;
    void		getPoints(OD::JSON::Array&, bool) const override;

    static const char*	sKeyTranslatorGrp()	{ return "Well"; }


protected:
    RefMan<Well::Data>	wd_;

};

mDeclareBaseBindings(Well, well)
mExternC(ODBind) hStringSet	well_lognames(hWell);
mExternC(ODBind) const char*	well_loginfo(hWell, const hStringSet);
mExternC(ODBind) hStringSet	well_markernames(hWell);
mExternC(ODBind) const char*	well_markerinfo(hWell, const hStringSet);
mExternC(ODBind) void		well_gettrack(hWell, hAllocator);
mExternC(ODBind) void		well_tvd(hWell, const float dah, float* tvd);
mExternC(ODBind) void		well_tvdss(hWell, const float dah,
					   float* tvdss);

// mExternC(ODBind) void		well_gettrack(hWell, hAllocator);
// mExternC(ODBind) void		well_getlogs(hWell, hAllocator, const hStringSet lognms, float zstep);

