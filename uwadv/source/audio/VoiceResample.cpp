//
// Underworld Adventures - an Ultima Underworld remake project
// Copyright (c) 2002,2003,2004,2005,2006,2019 Michael Fink
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
/// \file VoiceResample.cpp
/// \brief voc file resample implementation
//
#include "Audio.hpp"
#include <SDL_mixer.h>
#include "resamp.h"
#include <vector>

namespace Detail
{
   /// scale factor for FIR filter coefficients
   const double c_scale = 10.0;

   /// \brief FIR lowpass filter coefficients
   /// FIR lowpass filter
   /// Generated by FIRfilt(c)
   ///
   /// C/C++ format
   ///
   /// Edges (Hz):
   ///  0...11025
   ///  21622.98947...265056
   ///
   /// Specified deviations (dB):
   ///  1
   ///  -60
   ///
   /// Worst deviations (dB):
   ///  1.172906329
   ///  -58.44089729
   ///
   /// Target precision: IEEE 754 32 bit
   ///
   /// 88 coefficients:
   double c_coeff[88] =
   {
      c_scale * -9.273426038183077E-004,     /* h[0] */
      c_scale * -8.219791035608015E-004,     /* h[1] */
      c_scale * -1.161312424511615E-003,     /* h[2] */
      c_scale * -1.568537131500143E-003,     /* h[3] */
      c_scale * -2.045454770142925E-003,     /* h[4] */
      c_scale * -2.591167007955027E-003,     /* h[5] */
      c_scale * -3.200927102287597E-003,     /* h[6] */
      c_scale * -3.865148676380812E-003,     /* h[7] */
      c_scale * -4.574736705315427E-003,     /* h[8] */
      c_scale * -5.310939848155502E-003,     /* h[9] */
      c_scale * -6.056872314866817E-003,     /* h[10] */
      c_scale * -6.788473123302472E-003,     /* h[11] */
      c_scale * -7.479464924853740E-003,     /* h[12] */
      c_scale * -8.100542706882766E-003,     /* h[13] */
      c_scale * -8.619773439068060E-003,     /* h[14] */
      c_scale * -9.003444658047498E-003,     /* h[15] */
      c_scale * -9.218134767398807E-003,     /* h[16] */
      c_scale * -9.229278363867888E-003,     /* h[17] */
      c_scale * -9.004748433553839E-003,     /* h[18] */
      c_scale * -8.514022884489460E-003,     /* h[19] */
      c_scale * -7.730263941086692E-003,     /* h[20] */
      c_scale * -6.631536774404570E-003,     /* h[21] */
      c_scale * -5.201514512893354E-003,     /* h[22] */
      c_scale * -3.430479320035765E-003,     /* h[23] */
      c_scale * -1.316478002975218E-003,     /* h[24] */
      c_scale *  1.134949793158958E-003,     /* h[25] */
      c_scale *  3.909418615041150E-003,     /* h[26] */
      c_scale *  6.984028940547164E-003,     /* h[27] */
      c_scale *  1.032722754119598E-002,     /* h[28] */
      c_scale *  1.389890900433557E-002,     /* h[29] */
      c_scale *  1.765132404596670E-002,     /* h[30] */
      c_scale *  2.152991814950323E-002,     /* h[31] */
      c_scale *  2.547426494834237E-002,     /* h[32] */
      c_scale *  2.941972390411787E-002,     /* h[33] */
      c_scale *  3.329852066321085E-002,     /* h[34] */
      c_scale *  3.704145331541375E-002,     /* h[35] */
      c_scale *  4.057964442832007E-002,     /* h[36] */
      c_scale *  4.384612403464872E-002,     /* h[37] */
      c_scale *  4.677774874672057E-002,     /* h[38] */
      c_scale *  4.931689773898228E-002,     /* h[39] */
      c_scale *  5.141288335570159E-002,     /* h[40] */
      c_scale *  5.302346640288138E-002,     /* h[41] */
      c_scale *  5.411589717373218E-002,     /* h[42] */
      c_scale *  5.466777066559685E-002,     /* h[43] */
      c_scale *  5.466777066559685E-002,     /* h[44] */
      c_scale *  5.411589717373218E-002,     /* h[45] */
      c_scale *  5.302346640288138E-002,     /* h[46] */
      c_scale *  5.141288335570159E-002,     /* h[47] */
      c_scale *  4.931689773898228E-002,     /* h[48] */
      c_scale *  4.677774874672057E-002,     /* h[49] */
      c_scale *  4.384612403464872E-002,     /* h[50] */
      c_scale *  4.057964442832007E-002,     /* h[51] */
      c_scale *  3.704145331541375E-002,     /* h[52] */
      c_scale *  3.329852066321085E-002,     /* h[53] */
      c_scale *  2.941972390411787E-002,     /* h[54] */
      c_scale *  2.547426494834237E-002,     /* h[55] */
      c_scale *  2.152991814950323E-002,     /* h[56] */
      c_scale *  1.765132404596670E-002,     /* h[57] */
      c_scale *  1.389890900433557E-002,     /* h[58] */
      c_scale *  1.032722754119598E-002,     /* h[59] */
      c_scale *  6.984028940547164E-003,     /* h[60] */
      c_scale *  3.909418615041150E-003,     /* h[61] */
      c_scale *  1.134949793158958E-003,     /* h[62] */
      c_scale * -1.316478002975218E-003,     /* h[63] */
      c_scale * -3.430479320035765E-003,     /* h[64] */
      c_scale * -5.201514512893354E-003,     /* h[65] */
      c_scale * -6.631536774404570E-003,     /* h[66] */
      c_scale * -7.730263941086692E-003,     /* h[67] */
      c_scale * -8.514022884489460E-003,     /* h[68] */
      c_scale * -9.004748433553839E-003,     /* h[69] */
      c_scale * -9.229278363867888E-003,     /* h[70] */
      c_scale * -9.218134767398807E-003,     /* h[71] */
      c_scale * -9.003444658047498E-003,     /* h[72] */
      c_scale * -8.619773439068060E-003,     /* h[73] */
      c_scale * -8.100542706882766E-003,     /* h[74] */
      c_scale * -7.479464924853740E-003,     /* h[75] */
      c_scale * -6.788473123302472E-003,     /* h[76] */
      c_scale * -6.056872314866817E-003,     /* h[77] */
      c_scale * -5.310939848155502E-003,     /* h[78] */
      c_scale * -4.574736705315427E-003,     /* h[79] */
      c_scale * -3.865148676380812E-003,     /* h[80] */
      c_scale * -3.200927102287597E-003,     /* h[81] */
      c_scale * -2.591167007955027E-003,     /* h[82] */
      c_scale * -2.045454770142925E-003,     /* h[83] */
      c_scale * -1.568537131500143E-003,     /* h[84] */
      c_scale * -1.161312424511615E-003,     /* h[85] */
      c_scale * -8.219791035608015E-004,     /* h[86] */
      c_scale * -9.273426038183077E-004      /* h[87] */
   };

   /// interpolation factor
   const int c_interpolationFactor = 11;

   /// decimation factor
   const int c_decimationFactor = 6;

   /// \brief Resamples audio data from 12048 Hz to 22088 Hz
   /// The resample code from http://www.dspguru.com/sw/opendsp/alglib.htm was
   /// used, and the lowpass filter was designed with FIRfilt.
   ///
   /// The sample data is first upsampled to 132528 Hz, using the c_interpolationFactor
   /// factor and filtered with a FIR lowpass filter with coefficients c_coeff,
   /// which cutoff frequency is at 11025 Hz. As last step the signal is
   /// decimated by factor c_decimationFactor to 22088 Hz.
   ///
   /// \param sourceSamples source audio samples
   /// \param destSamples resampled audio
   void ResampleChunk12048_22050(const std::vector<double>& sourceSamples,
      std::vector<double>& destSamples)
   {
      unsigned int destSize = static_cast<unsigned int>(
         sourceSamples.size() * static_cast<double>(c_interpolationFactor) / c_decimationFactor + 1.0);
      destSamples.resize(destSize + 100, 0.0);

      // delay line for polyphase filter
      double delayLine[8] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

      // number of taps per polyphase filter
      int numTaps = SDL_TABLESIZE(c_coeff) / c_interpolationFactor;

      int currentPhase = 0;
      int outputSamples = 0;

      // resample
      resamp0(c_interpolationFactor, c_decimationFactor, numTaps,
         &currentPhase, c_coeff, delayLine,
         sourceSamples.size(), &sourceSamples[0],
         &destSamples[0], &outputSamples);

      // resize and remove last samples (could be distorted)
      destSamples.resize(outputSamples - SDL_TABLESIZE(delayLine));

      std::vector<double>(destSamples).swap(destSamples);
   }

} // namespace Detail
