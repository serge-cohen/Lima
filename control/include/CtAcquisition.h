//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2011
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################
#ifndef CTACQUISITION_H
#define CTACQUISITION_H

#include <algorithm>
#include <sstream>

#include "LimaCompatibility.h"
#include "Constants.h"
#include "HwInterface.h"
#include "HwCap.h"
#include "CtControl.h"
#include "CtConfig.h"
#include "Debug.h"

namespace lima 
{	

  class LIMACORE_API CtAcquisition 
  {
    DEB_CLASS_NAMESPC(DebModControl,"Acquisition","Control");
    friend class CtControl;

  public:
    enum AccTimeMode {Live,Real};
    enum AutoExposureMode {
      OFF,	  ///< Always off
      ON,	  ///< Always on
      ON_LIVE	  ///< ON during live and OFF for standard Acquisition
    };
    typedef std::vector<AutoExposureMode> AutoExposureModeList;

    struct LIMACORE_API Parameters {
      DEB_CLASS_NAMESPC(DebModControl,"Acquisition::Parameters","Control");
    public:
      Parameters();
      void reset();
      
      AcqMode		acqMode;
      AccTimeMode 	accTimeMode;
      int		acqNbFrames;
      double		acqExpoTime;
      double		accMaxExpoTime;
      int		concatNbFrames;
      double		latencyTime;
      TrigMode 		triggerMode;
      AutoExposureMode  autoExpoMode;
    };

    CtAcquisition(HwInterface *hw);
    virtual ~CtAcquisition();

    // --- global

    virtual void setPars(const Parameters &pars);
    virtual void getPars(Parameters& pars) const;

    virtual void reset();
    virtual void apply(CtControl::ApplyPolicy policy);
    virtual void sync();

    // --- acq modes

    virtual void setAcqMode(AcqMode mode);
    virtual void getAcqMode(AcqMode& mode) const;

    virtual void setAccTimeMode(AccTimeMode mode);
    virtual void getAccTimeMode(AccTimeMode &mode) const;

    virtual void setAcqNbFrames(int nframes);
    virtual void getAcqNbFrames(int& nframes) const;

    virtual void setAcqExpoTime(double acq_time);
    virtual void getAcqExpoTime(double& acq_time) const;
    bool checkAutoExposureMode(AutoExposureMode mode) const;
    virtual void getAutoExposureModeList(AutoExposureModeList& modes) const;
    virtual void setAutoExposureMode(AutoExposureMode mode);
    virtual void getAutoExposureMode(AutoExposureMode& mode) const;


    virtual void setAccMaxExpoTime(double max_time);
    virtual void getAccMaxExpoTime(double& max_time) const;

    virtual void getAccNbFrames(int& nframes) const;
    virtual void getAccExpoTime(double& acc_time) const;
    virtual void getAccLiveTime(double& acc_live_time) const;
    virtual void getAccDeadTime(double& acc_dead_time) const;

    virtual void setConcatNbFrames(int nframes);
    virtual void getConcatNbFrames(int& nframes) const; 

    // --- common

    virtual void setLatencyTime(double latency_time);
    virtual void getLatencyTime(double& latency_time) const;

    virtual void setTriggerMode(TrigMode mode);
    virtual void getTriggerMode(TrigMode& mode) const;

    virtual void getTriggerModeList(TrigModeList& modes) const;
  private:
    class _ValidRangesCallback;
    friend class _ValidRangesCallback;

    struct ChangedPars {
      DEB_CLASS_NAMESPC(DebModControl,"Acquisition::ChangedPars","Control");
    public:
      ChangedPars();
      void set(bool);
      void check(Parameters p1, Parameters p2);

      bool	acqExpoTime;
      bool	acqNbFrames;
      bool	latencyTime;
      bool	triggerMode;
      bool	accMaxExpoTime;
      bool	acqMode;
      bool      autoExpoMode;
    };

    void _updateAccPars() const;
    void _setDefaultPars(Parameters* pars);
    void _apply();
    virtual void _hwRead();

#ifdef WITH_CONFIG
    class _ConfigHandler;
    CtConfig::ModuleTypeCallback* _getConfigHandler();
#endif //WITH_CONFIG

    HwSyncCtrlObj	*m_hw_sync;
    HwSyncCtrlObj::ValidRangesType	m_valid_ranges;
    Parameters	m_inpars, m_hwpars;
    ChangedPars	m_changes;
    double		m_readout_time;
    double		m_frame_rate;
    mutable int		m_acc_nframes;
    mutable double	m_acc_exptime;
    mutable double	m_acc_live_time;
    mutable double	m_acc_dead_time;
    bool		m_applied_once;
    _ValidRangesCallback *m_valid_ranges_cb;
  };

  inline const char* convert_2_string(CtAcquisition::AccTimeMode accTimeMode)
    {
      const char *name = "Unknown";
      switch(accTimeMode)
	{
	case CtAcquisition::Live: name = "Live";
	case CtAcquisition::Real: name = "Real";
	}
      return name;
    }
  inline void convert_from_string(const std::string& val,
				  CtAcquisition::AccTimeMode& accTimeMode)
    {
      std::string buffer = val;
      std::transform(buffer.begin(),buffer.end(),
		     buffer.begin(),::tolower);
      if(buffer == "live")
	accTimeMode = CtAcquisition::Live;
      else if(buffer == "real")
	accTimeMode = CtAcquisition::Real;
      else
	{
	  std::ostringstream msg;
	  msg << "AccTimeMode can't be:" << DEB_VAR1(val);
	  throw LIMA_EXC(Control,InvalidValue,msg.str());
	}
    }
  inline std::ostream& operator<<(std::ostream &os,const CtAcquisition::Parameters &params)
  {
    os << "<"
       << "acqMode=" << params.acqMode << ", "
       << "acqNbFrames=" << params.acqNbFrames << ", "
       << "acqExpoTime=" << params.acqExpoTime << ", "
       << "accMaxExpoTime=" << params.accMaxExpoTime << ", "
       << "concatNbFrames=" << params.concatNbFrames << ", "
       << "latencyTime=" << params.latencyTime << ", "
       << "triggerMode=" << params.triggerMode << ", "
       << "autoExpoMode=" << params.autoExpoMode
       << ">";
    return os; 
  }

  inline const char* convert_2_string(CtAcquisition::AutoExposureMode mode)
  {
    const char* aHumanReadablePt;
    switch(mode)
      {
      case CtAcquisition::OFF:		aHumanReadablePt = "OFF";	break;
      case CtAcquisition::ON:		aHumanReadablePt = "ON";	break;
      case CtAcquisition::ON_LIVE:	aHumanReadablePt = "ON LIVE";	break;
      default:
	aHumanReadablePt = "UNKNOWN";
	break;
      }
    return aHumanReadablePt;
  }

  inline void convert_from_string(const std::string& val,
				  CtAcquisition::AutoExposureMode& mode)
  {
    std::string buffer = val;
    std::transform(buffer.begin(),buffer.end(),
		   buffer.begin(),::tolower);
    
    if(buffer == "off") mode = CtAcquisition::OFF;
    else if(buffer == "on") mode = CtAcquisition::ON;
    else if(buffer == "on live") mode = CtAcquisition::ON_LIVE;
    else
      {
	std::ostringstream msg;
	msg << "AutoExposureMode can't be:" << DEB_VAR1(val);
	throw LIMA_EXC(Common,InvalidValue,msg.str());
      }
  }
  inline std::ostream& operator<<(std::ostream& os,
				  const CtAcquisition::AutoExposureMode& mode)
  {
    return os << convert_2_string(mode);
  }

  
} // namespace lima

#endif // CTACQUISITION_H
