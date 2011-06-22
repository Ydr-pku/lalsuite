/*
 * Copyright (C) 2007 Jolien Creighton, Patrick Brady, Saikat Ray-Majumder,
 * Xavier Siemens, Teviet Creighton, Kipp Cannon
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with with program; see the file COPYING. If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */


/*
 * ============================================================================
 *
 *                                  Preamble
 *
 * ============================================================================
 */


#include <math.h>
#include <string.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <lal/Date.h>
#include <lal/GenerateBurst.h>
#include <lal/Units.h>
#include <lal/LALComplex.h>
#include <lal/LALConstants.h>
#include <lal/LALDatatypes.h>
#include <lal/LALDetectors.h>
#include <lal/LALSimBurst.h>
#include <lal/LALSimulation.h>
#include <lal/LIGOMetadataTables.h>
#include <lal/TimeFreqFFT.h>
#include <lal/TimeSeries.h>
#include <lal/FrequencySeries.h>


NRCSID(GENERATEBURSTC, "$Id$");


/*
 * ============================================================================
 *
 *                              sim_burst Nexus
 *
 * ============================================================================
 */


/*
 * Generate the + and x time series for a single sim_burst table row.
 * Note:  only the row pointed to by sim_burst is processed, the linked
 * list is not iterated over.
 */


int XLALGenerateSimBurst(
	REAL8TimeSeries **hplus,
	REAL8TimeSeries **hcross,
	const SimBurst *sim_burst,
	double delta_t
)
{
	if(!strcmp(sim_burst->waveform, "BTLWNB")) {
		/* E_{GW}/r^{2} is in M_{sun} / pc^{2}, so we multiply by
		 * (M_{sun} c^2) to convert to energy/pc^{2}, and divide by
		 * (distance/pc)^{2} to convert to energy/distance^{2},
		 * which is then multiplied by (4 G / c^3) to convert to a
		 * value of \int \dot{h}^{2} \diff t.  From the values of
		 * the LAL constants, the total factor multiplying
		 * egw_over_rsquared is 1.8597e-21. */

		double int_hdot_squared_dt = sim_burst->egw_over_rsquared * LAL_MSUN_SI * 4 * LAL_G_SI / LAL_C_SI / LAL_PC_SI / LAL_PC_SI;

		/* the waveform number is interpreted as the seed for GSL's
		 * Mersenne twister random number generator */
		gsl_rng *rng = gsl_rng_alloc(gsl_rng_mt19937);

		if(!rng) {
			XLALPrintError("%s(): failure creating random number generator\n", __func__);
			XLAL_ERROR(__func__, XLAL_ENOMEM);
		}
		gsl_rng_set(rng, sim_burst->waveform_number);

		XLALPrintInfo("%s(): BTLWNB @ %9d.%09u: f = %.16g Hz, df = %.16g Hz, dt = %.16g s, hdot^2 = %.16g\n", __func__, sim_burst->time_geocent_gps.gpsSeconds, sim_burst->time_geocent_gps.gpsNanoSeconds, sim_burst->frequency, sim_burst->bandwidth, sim_burst->duration, int_hdot_squared_dt);
		if(XLALGenerateBandAndTimeLimitedWhiteNoiseBurst(hplus, hcross, sim_burst->duration, sim_burst->frequency, sim_burst->bandwidth, int_hdot_squared_dt, delta_t, rng)) {
			gsl_rng_free(rng);
			XLAL_ERROR(__func__, XLAL_EFUNC);
		}
		gsl_rng_free(rng);
	} else if(!strcmp(sim_burst->waveform, "StringCusp")) {
	  XLALPrintInfo("%s(): string cusp @ %9d.%09u: A = %.16g, fhigh = %.16g Hz\n", __func__, sim_burst->time_geocent_gps.gpsSeconds, sim_burst->time_geocent_gps.gpsNanoSeconds, sim_burst->amplitude, sim_burst->frequency);
		if(XLALGenerateStringCusp(hplus, hcross, sim_burst->amplitude, sim_burst->frequency, delta_t))
			XLAL_ERROR(__func__, XLAL_EFUNC);
	} else if(!strcmp(sim_burst->waveform, "SineGaussian")) {
		XLALPrintInfo("%s(): sine-Gaussian @ %9d.%09u: f = %.16g Hz, Q = %.16g, hrss = %.16g\n", __func__, sim_burst->time_geocent_gps.gpsSeconds, sim_burst->time_geocent_gps.gpsNanoSeconds, sim_burst->frequency, sim_burst->q, sim_burst->hrss);
		if(XLALSimBurstSineGaussian(hplus, hcross, sim_burst->q, sim_burst->frequency, sim_burst->hrss, sim_burst->pol_ellipse_e, sim_burst->pol_ellipse_angle, delta_t))
			XLAL_ERROR(__func__, XLAL_EFUNC);
	} else if(!strcmp(sim_burst->waveform, "Impulse")) {
		XLALPrintInfo("%s(): impulse @ %9d.%09u: hpeak = %.16g\n", __func__, sim_burst->time_geocent_gps.gpsSeconds, sim_burst->time_geocent_gps.gpsNanoSeconds, sim_burst->amplitude, delta_t);
		if(XLALGenerateImpulseBurst(hplus, hcross, sim_burst->amplitude, delta_t))
			XLAL_ERROR(__func__, XLAL_EFUNC);
	} else {
		/* unrecognized waveform */
		XLALPrintError("%s(): error: unrecognized waveform\n", __func__);
		XLAL_ERROR(__func__, XLAL_EINVAL);
	}

	/* done */

	return 0;
}


/**
 * Find the first element in the linked list of TimeSlide objects whose
 * time_slide_id and instrument name match the values given.  NOTE:  a
 * TimeSlide object's instrument is considered to match the requested
 * instrument name as long as it matches the first characters in the
 * requested instrument name.  This has been done to allow a channel name
 * like "H1:LSC-STRAIN" to be passed to this function without having to
 * explicitly construct a new string containing just the instrument part of
 * the channel name.  TimeSlide objects with no instrument name set or
 * whose instrument name is a zero-length string are ignored.  Returns NULL
 * if no matching row is found.
 */


static const TimeSlide *XLALTimeSlideGetByIDAndInstrument(const TimeSlide *time_slide, long time_slide_id, const char *instrument)
{
	for(; time_slide && time_slide->time_slide_id != time_slide_id && time_slide->instrument && strlen(time_slide->instrument) && strncmp(instrument, time_slide->instrument, strlen(time_slide->instrument)); time_slide = time_slide->next);
	return time_slide;
}


/*
 * Convenience wrapper to iterate over the entries in a sim_burst linked
 * list and inject them into a time series.  Passing NULL for the response
 * disables it (input time series is strain).
 */


int XLALBurstInjectSignals(
	REAL8TimeSeries *series,
	const SimBurst *sim_burst,
	const TimeSlide *time_slide_table_head,
	const COMPLEX16FrequencySeries *response
)
{
	/* to be deduced from the time series' channel name */
	const LALDetector *detector;
	/* FIXME:  fix the const entanglement so as to get rid of this */
	LALDetector detector_copy;
	/* + and x time series for injection waveform */
	REAL8TimeSeries *hplus, *hcross;
	/* injection time series as added to detector's */
	REAL8TimeSeries *h;
	/* skip injections whose geocentre times are more than this many
	 * seconds outside of the target time series */
	const double injection_window = 100.0;

	/* turn the first two characters of the channel name into a
	 * detector */

	detector = XLALInstrumentNameToLALDetector(series->name);
	if(!detector)
		XLAL_ERROR(__func__, XLAL_EFUNC);
	XLALPrintInfo("%s(): channel name is '%s', instrument appears to be '%s'\n", __func__, series->name, detector->frDetector.prefix);
	detector_copy = *detector;

	/* iterate over injections */

	for(; sim_burst; sim_burst = sim_burst->next) {
		LIGOTimeGPS time_geocent_gps;
		const TimeSlide *time_slide_row;

		/* determine the offset to be applied to this injection */

		time_slide_row = XLALTimeSlideGetByIDAndInstrument(time_slide_table_head, sim_burst->time_slide_id, series->name);
		if(!time_slide_row) {
			XLALPrintError("%s(): cannot find time shift offset for injection 'sim_burst:simulation_id:%ld'.  need 'time_slide:time_slide_id:%ld' for instrument for channel '%s'", __func__, sim_burst->simulation_id, sim_burst->time_slide_id, series->name);
			XLAL_ERROR(__func__, XLAL_EINVAL);
		}

		XLALPrintInfo("%s(): shifting injection 'sim_burst:simulation_id:%ld' in '%s' at %d.%09u s by %.16g s\n", __func__, sim_burst->simulation_id, series->name, sim_burst->time_geocent_gps.gpsSeconds, sim_burst->time_geocent_gps.gpsNanoSeconds, -time_slide_row->offset);
		time_geocent_gps = sim_burst->time_geocent_gps;
		XLALGPSAdd(&time_geocent_gps, -time_slide_row->offset);

		/* skip injections whose "times" are too far outside of the
		 * target time series */

		if(XLALGPSDiff(&series->epoch, &time_geocent_gps) > injection_window || XLALGPSDiff(&time_geocent_gps, &series->epoch) > (series->data->length * series->deltaT + injection_window))
			continue;

		/* construct the h+ and hx time series for the injection
		 * waveform.  in the time series produced by this function,
		 * t = 0 is the "time" of the injection. */

		if(XLALGenerateSimBurst(&hplus, &hcross, sim_burst, series->deltaT))
			XLAL_ERROR(__func__, XLAL_EFUNC);

		/* add the time of the injection at the geocentre to the
		 * start times of the h+ and hx time series.  after this,
		 * their epochs mark the start of those time series at the
		 * geocentre. */

		XLALGPSAddGPS(&hcross->epoch, &time_geocent_gps);
		XLALGPSAddGPS(&hplus->epoch, &time_geocent_gps);

		/* project the wave onto the detector to produce the strain
		 * in the detector. */

		h = XLALSimDetectorStrainREAL8TimeSeries(hplus, hcross, sim_burst->ra, sim_burst->dec, sim_burst->psi, &detector_copy);
		XLALDestroyREAL8TimeSeries(hplus);
		XLALDestroyREAL8TimeSeries(hcross);
		if(!h)
			XLAL_ERROR(__func__, XLAL_EFUNC);

		/* add the injection strain time series to the detector
		 * data */

		if(XLALSimAddInjectionREAL8TimeSeries(series, h, response)) {
			XLALDestroyREAL8TimeSeries(h);
			XLAL_ERROR(__func__, XLAL_EFUNC);
		}
		XLALDestroyREAL8TimeSeries(h);
	}

	/* done */

	return 0;
}
