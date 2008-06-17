/****************************************************************************
 *
 * $Id: manGrabOSXcfox.cpp,v 1.5 2008-06-17 08:54:22 asaunier Exp $
 *
 * Copyright (C) 1998-2008 Inria. All rights reserved.
 *
 * This software was developed at:
 * IRISA/INRIA Rennes
 * Projet Lagadic
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * http://www.irisa.fr/lagadic
 *
 * This file is part of the ViSP toolkit
 *
 * This file may be distributed under the terms of the Q Public License
 * as defined by Trolltech AS of Norway and appearing in the file
 * LICENSE included in the packaging of this file.
 *
 * Licensees holding valid ViSP Professional Edition licenses may
 * use this file in accordance with the ViSP Commercial License
 * Agreement provided with the Software.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Contact visp@irisa.fr if any conditions of this licensing are
 * not clear to you.
 *
 * Description:
 * Images grabbing example.
 *
 * Authors:
 * Anthony Saunier
 * Fabien Spindler
 *
 *****************************************************************************/
/*!
  \file manGrabOSXcfox.cpp

  \brief Images grabbing example with the vpOSXcfoxGrabber class.

 */
/*!
  \example manGrabOSXcfox.cpp

  \brief Images grabbing example with the vpOSXcfoxGrabber class.
 
 */

#include <visp/vpConfig.h>

#include <stdlib.h>

#include <visp/vpImage.h>
#include <visp/vpOSXcfoxGrabber.h>

int main()
{
  vpImage<unsigned char> I; // Grey level image
  
#ifdef VISP_HAVE_CFOX
  vpOSXcfoxGrabber g;
  g.setInput(2);    // Input 2 on the board
  g.setScale(2);    // Use half-size images
  g.open(I);        // Open the grabber
  while(1)
    g.acquire(I);     // Acquire an image
#endif

  return 0;
}
