/****************************************************************************
 *
 * $Id: vpRobotPtu46.cpp,v 1.5 2006-05-30 08:40:45 fspindle Exp $
 *
 * Copyright (C) 1998-2006 Inria. All rights reserved.
 *
 * This software was developed at:
 * IRISA/INRIA Rennes
 * Projet Lagadic
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * http://www.irisa.fr/lagadic
 *
 * This file is part of the ViSP toolkit.
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
 * Interface for the ptu-46 robot.
 *
 * Authors:
 * Fabien Spindler
 *
 *****************************************************************************/

#include <visp/vpConfig.h>
#ifdef VISP_HAVE_PTU46

/* Headers des fonctions implementees. */
#include <visp/vpPtu46.h>
#include <visp/vpRobotPtu46.h>
#include <visp/vpRobotException.h>
#include <visp/vpDebug.h>


#include <signal.h>

/* ---------------------------------------------------------------------- */
/* --- STATIC ------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

bool vpRobotPtu46::robotAlreadyCreated = false;
const double vpRobotPtu46::defaultPositioningVelocity = 10.0;

/* ----------------------------------------------------------------------- */
/* --- CONSTRUCTOR ------------------------------------------------------ */
/* ---------------------------------------------------------------------- */


/*!

  Default constructor.

  Initialize the ptu-46 pan, tilt head by opening the serial port.

  \sa init()

*/
vpRobotPtu46::vpRobotPtu46 (const char *device)
  :
  vpRobot ()
{
  device = new char [FILENAME_MAX];

  sprintf(this->device, "%s", device);

  DEBUG_TRACE (12, "Open communication with Ptu-46.");
  init();

  try
  {
    setRobotState(vpRobot::STATE_STOP) ;
  }
  catch(...)
  {
    ERROR_TRACE(" ") ;
    throw ;
  }
  positioningVelocity = defaultPositioningVelocity ;
  return ;
}

/* ------------------------------------------------------------------------ */
/* --- DESTRUCTOR  -------------------------------------------------------- */
/* ------------------------------------------------------------------------ */

/*!

  Destructor.
  Close the serial connection with the head.

*/
vpRobotPtu46::~vpRobotPtu46 (void)
{

  setRobotState(vpRobot::STATE_STOP) ;

  if (0 != ptu.close())
  {
    ERROR_TRACE ("Error while closing communications with the robot ptu-46.");
  }

  vpRobotPtu46::robotAlreadyCreated = false;

  delete [] device;

  return;
}


/* -------------------------------------------------------------------------- */
/* --- INITIALISATION ------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/*!

  Open the serial port.

  \param device : Serial port.

  \exception vpRobotException::constructionError If the device cannot be
  oppened.

*/
void
vpRobotPtu46::init ()
{

  DEBUG_TRACE (12, "Open connection Ptu-46.");
  if (0 != ptu.init(device) )
  {
    ERROR_TRACE ("Cannot open connexion with ptu-46.");
    throw vpRobotException (vpRobotException::constructionError,
			    "Cannot open connexion with ptu-46");
  }

  return ;
}


/*!

  Change the state of the robot either to stop them, or to set position or
  speed control.

*/
vpRobot::RobotStateType
vpRobotPtu46::setRobotState(vpRobot::RobotStateType newState)
{
  switch (newState)
  {
  case vpRobot::STATE_STOP:
    {
      if (vpRobot::STATE_STOP != getRobotState ())
      {
	ptu.stop();
      }
      break;
    }
  case vpRobot::STATE_POSITION_CONTROL:
    {
      if (vpRobot::STATE_VELOCITY_CONTROL  == getRobotState ())
      {
	DEBUG_TRACE (12, "Passage vitesse -> position.");
	ptu.stop();
      }
      else
      {
	DEBUG_TRACE (1, "Passage arret -> position.");
      }
      break;
    }
  case vpRobot::STATE_VELOCITY_CONTROL:
    {
      if (vpRobot::STATE_POSITION_CONTROL != getRobotState ())
      {
	DEBUG_TRACE (10, "Arret du robot...");
	ptu.stop();
      }
      break;
    }
  default:
    break ;
  }

  return vpRobot::setRobotState (newState);
}

/*!

  Halt all the axis.

*/
void
vpRobotPtu46::stopMotion(void)
{
  ptu.stop();
  setRobotState (vpRobot::STATE_STOP);
}


/*!

  Get the twist matrix corresponding to the transformation between the
  camera frame and the end effector frame. The end effector frame is located on
  the tilt axis.

  \param cVe : Twist transformation between camera and end effector frame to
  expess a velocity skew from end effector frame in camera frame.

*/
void
vpRobotPtu46::get_cVe(vpTwistMatrix &cVe)
{
  vpHomogeneousMatrix cMe ;
  vpPtu46::get_cMe(cMe) ;

  cVe.buildFrom(cMe) ;
}

/*!

  Get the homogeneous matrix corresponding to the transformation between the
  camera frame and the end effector frame. The end effector frame is located on
  the tilt axis.

  \param cMe :  Homogeneous matrix between camera and end effector frame.

*/
void
vpRobotPtu46::get_cMe(vpHomogeneousMatrix &cMe)
{
  vpPtu46::get_cMe(cMe) ;
}

/*!
  Get the robot jacobian expressed in the end-effector frame.

  \warning Re is not the embedded camera frame. It corresponds to the frame
  associated to the tilt axis (see also get_cMe).

  \param eJe : Jacobian between end effector frame and end effector frame (on
  tilt axis).

*/
void
vpRobotPtu46::get_eJe(vpMatrix &eJe)
{
  vpColVector q(2) ;
  getPosition(vpRobot::ARTICULAR_FRAME, q) ;

  try
  {
    vpPtu46::get_eJe(q,eJe) ;
  }
  catch(...)
  {
    ERROR_TRACE("catch exception ") ;
    throw ;
  }
}

/*!
  Get the robot jacobian expressed in the robot reference frame

  \param fJe : Jacobian between reference frame (or fix frame) and end effector
  frame (on tilt axis).

*/
void
vpRobotPtu46::get_fJe(vpMatrix &fJe)
{
  vpColVector q(2) ;
  getPosition(vpRobot::ARTICULAR_FRAME, q) ;

  try
  {
    vpPtu46::get_fJe(q,fJe) ;
  }
  catch(...)
  {
    throw ;
  }

}


/*!

  Set the velocity for a positionning task

  \param velocity : Velocity in % of the maximum velocity between [0,100].
*/
void
vpRobotPtu46::setPositioningVelocity (const double velocity)
{
  positioningVelocity = velocity;
}
/*!
  Get the velocity in % for a positionning task.

  \return Positionning velocity in [0, 100]

*/
double
vpRobotPtu46::getPositioningVelocity (void)
{
  return positioningVelocity;
}


/*!
   Move the robot in position control.

   \warning This method is blocking. That mean that it waits the end of the
   positionning.

   \param frame : Control frame. This head can only be controlled in
   articular.

   \param q : The position to set for each axis.

   \exception vpRobotException::wrongStateError : If a not supported frame type
   is given.

*/

void
vpRobotPtu46::setPosition (const vpRobot::ControlFrameType frame,
			   const vpColVector & q )
{

  if (vpRobot::STATE_POSITION_CONTROL != getRobotState ())
  {
    ERROR_TRACE ("Robot was not in position-based control\n"
		 "Modification of the robot state");
    setRobotState(vpRobot::STATE_POSITION_CONTROL) ;
  }

  switch(frame)
  {
  case vpRobot::CAMERA_FRAME:
    ERROR_TRACE ("Cannot move the robot in camera frame: "
		 "not implemented");
    throw vpRobotException (vpRobotException::wrongStateError,
			    "Cannot move the robot in camera frame: "
			    "not implemented");
    break;
  case vpRobot::REFERENCE_FRAME:
    ERROR_TRACE ("Cannot move the robot in reference frame: "
		 "not implemented");
    throw vpRobotException (vpRobotException::wrongStateError,
			    "Cannot move the robot in reference frame: "
			    "not implemented");
    break;
  case vpRobot::MIXT_FRAME:
    ERROR_TRACE ("Cannot move the robot in mixt frame: "
		 "not implemented");
    throw vpRobotException (vpRobotException::wrongStateError,
			    "Cannot move the robot in mixt frame: "
			    "not implemented");
    break;
  case vpRobot::ARTICULAR_FRAME:
    break ;
  }

  // Interface for the controller
  double artpos[2];

  artpos[0] = q[0];
  artpos[1] = q[1];

  if (0 != ptu.move(artpos, positioningVelocity, PTU_ABSOLUTE_MODE) )
  {
    ERROR_TRACE ("Positionning error.");
    throw vpRobotException (vpRobotException::lowLevelError,
			    "Positionning error.");
  }

  return ;
}


/*!
   Move the robot in position control.

   \warning This method is blocking. That mean that it wait the end of the
   positionning.

   \param frame : Control frame. This head can only be controlled in
   articular.

   \param q1 : The pan position to set.
   \param q2 : The tilt position to set.

   \exception vpRobotException::wrongStateError : If a not supported frame type
   is given.

*/
void
vpRobotPtu46::setPosition (const vpRobot::ControlFrameType frame,
			   const double &q1, const double &q2)
{
  try{
    vpColVector q(2) ;
    q[0] = q1 ;
    q[1] = q2 ;

    setPosition(frame,q) ;
  }
  catch(...)
  {
    throw ;
  }
}

/*!

  Read the content of the position file and moves to head to articular
  position.

  \param filename : Position filename

  \exception vpRobotException::readingParametersError : If the articular
  position cannot be read from file.

  \sa readPositionFile()

*/
void
vpRobotPtu46::setPosition(const char *filename)
{
  vpColVector q ;
  if (readPositionFile(filename, q) == false) {
    ERROR_TRACE ("Cannot get ptu-46 position from file");
    throw vpRobotException (vpRobotException::readingParametersError,
			    "Cannot get ptu-46 position from file");
  }
  setPosition ( vpRobot::ARTICULAR_FRAME, q) ;
}

/*!

  Return the position of each axis.

  \param frame : Control frame. This head can only be controlled in
  articular.

  \param q : The position of the axis.

  \exception vpRobotException::wrongStateError : If a not supported frame type
  is given.

*/
void
vpRobotPtu46::getPosition (const vpRobot::ControlFrameType frame,
			   vpColVector & q)
{
  DEBUG_TRACE (9, "# Entree.");

  switch(frame)
  {
  case vpRobot::CAMERA_FRAME :
    ERROR_TRACE ("Cannot get position in camera frame: not implemented");
    throw vpRobotException (vpRobotException::lowLevelError,
			    "Cannot get position in camera frame: "
			    "not implemented");
    break;
  case vpRobot::REFERENCE_FRAME:
    ERROR_TRACE ("Cannot get position in reference frame: "
		 "not implemented");
    throw vpRobotException (vpRobotException::lowLevelError,
			    "Cannot get position in reference frame: "
			    "not implemented");
    break;
  case vpRobot::MIXT_FRAME:
    ERROR_TRACE ("Cannot get position in mixt frame: "
		 "not implemented");
    throw vpRobotException (vpRobotException::lowLevelError,
			    "Cannot get position in mixt frame: "
			    "not implemented");
    break;
  case vpRobot::ARTICULAR_FRAME:
    break ;
  }

  double artpos[2];

  if (0 != ptu.getCurrentPosition( artpos ) )
  {
    ERROR_TRACE ("Error when calling  recup_posit_Afma4.");
    throw vpRobotException (vpRobotException::lowLevelError,
			    "Error when calling  recup_posit_Afma4.");
  }

  q.resize (vpPtu46::ndof);

  q[0] = artpos[0];
  q[1] = artpos[1];
}


/*!

  Send a velocity on each axis.

  \param frame : Control frame. This head can only be controlled in articular
  and camera frame. Be aware, the reference frame and the mixt frame are not
  implemented

  \param v : The desired velocity of the axis.

  \warning In camera frame, we control only the rx and ry camera velocities;
  v dimension must be two: v[0] correspond to rx, and v[1] to ry

  \waning In articular, we control the 2 dof, v[0] corresponds the pan velocity
  and v[1] tilt velocity (in radians)

  \exception vpRobotException::wrongStateError : If a not supported frame type
  is given.
*/

void
vpRobotPtu46::setVelocity (const vpRobot::ControlFrameType frame,
			   const vpColVector & v)
{
  TPtuFrame ptuFrameInterface;

  if (vpRobot::STATE_VELOCITY_CONTROL != getRobotState ())
  {
    ERROR_TRACE ("Cannot send a velocity to the robot "
		 "use setRobotState(vpRobot::STATE_VELOCITY_CONTROL) first) ");
    throw vpRobotException (vpRobotException::wrongStateError,
			    "Cannot send a velocity to the robot "
			    "use setRobotState(vpRobot::STATE_VELOCITY_CONTROL) first) ");
  }

  switch(frame)
  {
  case vpRobot::CAMERA_FRAME :
    {
      ptuFrameInterface = PTU_CAMERA_FRAME;
      if ( v.getRows() != 2) {
	ERROR_TRACE ("Bad dimension fo speed vector in camera frame");
	throw vpRobotException (vpRobotException::wrongStateError,
				"Bad dimension for speed vector "
				"in camera frame");
      }
      break ;
    }
  case vpRobot::ARTICULAR_FRAME :
    {
      ptuFrameInterface = PTU_ARTICULAR_FRAME;
      if ( v.getRows() != 2) {
	ERROR_TRACE ("Bad dimension fo speed vector in articular frame");
	throw vpRobotException (vpRobotException::wrongStateError,
				"Bad dimension for speed vector "
				"in articular frame");
      }
      break ;
    }
  case vpRobot::REFERENCE_FRAME :
    {
      ERROR_TRACE ("Cannot send a velocity to the robot "
		   "in the reference frame: "
		   "functionality not implemented");
      throw vpRobotException (vpRobotException::wrongStateError,
			      "Cannot send a velocity to the robot "
			      "in the reference frame:"
			      "functionality not implemented");
      break ;
    }
  case vpRobot::MIXT_FRAME :
    {
      ERROR_TRACE ("Cannot send a velocity to the robot "
		   "in the mixt frame: "
		   "functionality not implemented");
      throw vpRobotException (vpRobotException::wrongStateError,
			      "Cannot send a velocity to the robot "
			      "in the mixt frame:"
			      "functionality not implemented");
      break ;
    }
  default:
    {
      ERROR_TRACE ("Error in spec of vpRobot. "
		   "Case not taken in account.");
      throw vpRobotException (vpRobotException::wrongStateError,
			      "Cannot send a velocity to the robot ");
    }
  }

  DEBUG_TRACE (12, "Velocity limitation.");
  double ptuSpeedInterface[2];

  switch(frame) {
  case vpRobot::ARTICULAR_FRAME :
  case vpRobot::CAMERA_FRAME : {
    double max = this ->maxRotationVelocity;
    for (int i = 0 ; i < 2; ++ i) // rx and ry of the camera
    {
      if (fabs (v[i]) > max)
      {
	max = fabs (v[i]);
	ERROR_TRACE ("Excess velocity: ROTATION "
		     "(axe nr.%d).", i);
      }
    }
    max =  this ->maxRotationVelocity / max;
    for (int i = 0 ; i < 2; ++ i)
     ptuSpeedInterface [i] = v[i]*max;

    break;
  }
  default:
    // Should never occur
    break;

  }

  CDEBUG(12) << "v: " << ptuSpeedInterface[0]
	     << " " << ptuSpeedInterface[1] << endl;
  ptu.move(ptuSpeedInterface, ptuFrameInterface);
  return;
}


/* ------------------------------------------------------------------------- */
/* --- GET ----------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */


/*!

  Get the articular velocity.

  \param frame : Control frame. This head can only be controlled in articular.

  \param q_dot : The measured articular velocity in rad/s.

  \exception vpRobotException::wrongStateError : If a not supported frame type
  is given.
*/
void
vpRobotPtu46::getVelocity (const vpRobot::ControlFrameType frame,
			   vpColVector & q_dot)
{

  TPtuFrame ptuFrameInterface = PTU_ARTICULAR_FRAME;

  switch (frame)
  {
  case vpRobot::CAMERA_FRAME:
    {
      ERROR_TRACE ("Cannot get a velocity in the camera frame: "
		   "functionality not implemented");
      throw vpRobotException (vpRobotException::wrongStateError,
			      "Cannot get a velocity in the camera frame:"
			      "functionality not implemented");
       break ;
    }
  case vpRobot::ARTICULAR_FRAME:
    {
      ptuFrameInterface = PTU_ARTICULAR_FRAME;
      break ;
    }
  case vpRobot::REFERENCE_FRAME:
    {
      ERROR_TRACE ("Cannot get a velocity in the reference frame: "
		   "functionality not implemented");
      throw vpRobotException (vpRobotException::wrongStateError,
			      "Cannot get a velocity in the reference frame:"
			      "functionality not implemented");
      break ;
    }
  case vpRobot::MIXT_FRAME:
    {

      ERROR_TRACE ("Cannot get a velocity in the mixt frame: "
		   "functionality not implemented");
      throw vpRobotException (vpRobotException::wrongStateError,
			      "Cannot get a velocity in the mixt frame:"
			      "functionality not implemented");
      break ;
    }
  }

  q_dot.resize(vpPtu46::ndof);
  double ptuSpeedInterface[2];

  ptu.getCurrentSpeed(ptuSpeedInterface, ptuFrameInterface);

  q_dot[0] = ptuSpeedInterface[0];
  q_dot[1] = ptuSpeedInterface[1];

}


/*!

  Return the articular velocity.

  \param frame : Control frame. This head can only be controlled in articular.

  \return The measured articular velocity in rad/s.

  \exception vpRobotException::wrongStateError : If a not supported frame type
  is given.
*/
vpColVector
vpRobotPtu46::getVelocity (vpRobot::ControlFrameType frame)
{
  vpColVector q_dot;
  getVelocity (frame, q_dot);

  return q_dot;
}

/*!

  Get an articular position from the position file.

  \param filename : Position file.

  \param q : The articular position read in the file.

  \code
  # Example of ptu-46 position file
  # The axis positions must be preceed by R:
  # First value : pan  articular position in degrees
  # Second value: tilt articular position in degrees
  R: 15.0 5.0
  \endcode

  \return true if a position was found, false otherwise.

*/
bool
vpRobotPtu46::readPositionFile(const char *filename, vpColVector &q)
{
  FILE * pt_f ;
  pt_f = fopen(filename,"r") ;

  if (pt_f == NULL) {
    ERROR_TRACE ("Can not open ptu-46 position file %s", filename);
    return false;
  }

  char line[FILENAME_MAX];
  char head[] = "R:";
  bool end = false;

  do {
    // skip lines begining with # for comments
    if (fgets (line, 100, pt_f) != NULL) {
      if ( strncmp (line, "#", 1) != 0) {
	// this line is not a comment
	if ( fscanf (pt_f, "%s", line) != EOF)   {
	  if ( strcmp (line, head) == 0)
	    end = true; 	// robot position was found
	}
	else
	  return (false); // end of file without position
      }
    }
    else {
      return (false);// end of file
    }

  }
  while ( end != true );

  double q1,q2;
  // Read positions
  fscanf(pt_f, "%lf %lf", &q1, &q2);
  q.resize(vpPtu46::ndof) ;

  q[0] = vpMath::rad(q1) ; // Rot tourelle
  q[1] = vpMath::rad(q2) ;

  fclose(pt_f) ;
  return (true);
}

/*!

  Get the robot displacement expressed in the camera frame since the last call
  of this method.

  \param v The measured displacement in camera frame. The dimension of v is 6
  (tx, ty, ty, rx, ry, rz). Translations are expressed in meters, rotations in
  radians.

  \sa getDisplacement(), getArticularDisplacement()

*/
void
vpRobotPtu46::getCameraDisplacement(vpColVector &v)
{
  getDisplacement(vpRobot::CAMERA_FRAME, v);

}
/*!

  Get the robot articular displacement since the last call of this method.

  \param d The measured articular displacement. The dimension of d is 2 (the
  number of axis of the robot) with respectively d[0] (pan displacement),
  d[1] (tilt displacement)

  \sa getDisplacement(), getCameraDisplacement()

*/
void vpRobotPtu46::getArticularDisplacement(vpColVector  &d)
{
  getDisplacement(vpRobot::ARTICULAR_FRAME, d);
}

/*!

  Get the robot displacement since the last call of this method.

  \warning The first call of this method gives not a good value for the
  displacement.

  \param frame The frame in which the measured displacement is expressed.

  \param d The displacement:
  - In articular, the dimension of q is 2  (the number of axis of the robot)
  with respectively d[0] (pan displacement), d[1] (tilt displacement).
  - In camera frame, the dimension of d is 6 (tx, ty, ty, rx, ry,
  rz). Translations are expressed in meters, rotations in radians.

  \exception vpRobotException::wrongStateError If a not supported frame type is
  given.

  \sa getArticularDisplacement(), getCameraDisplacement()

*/
void
vpRobotPtu46::getDisplacement(vpRobot::ControlFrameType frame,
			      vpColVector &d)
{
  double d_[6];

  switch (frame)
  {
  case vpRobot::CAMERA_FRAME:
    {
      d.resize (6);
      ptu.measureDpl(d_, PTU_CAMERA_FRAME);
      d[0]=d_[0];
      d[1]=d_[1];
      d[2]=d_[2];
      d[3]=d_[3];
      d[4]=d_[4];
      d[5]=d_[5];
      break ;
    }
  case vpRobot::ARTICULAR_FRAME:
    {
      ptu.measureDpl(d_, PTU_ARTICULAR_FRAME);
      d.resize (vpPtu46::ndof);
      d[0]=d_[0];  // pan
      d[1]=d_[1];  // tilt
      break ;
    }
  case vpRobot::REFERENCE_FRAME:
    {
      ERROR_TRACE ("Cannot get a displacement in the reference frame: "
		   "functionality not implemented");
      throw vpRobotException (vpRobotException::wrongStateError,
			      "Cannot get a displacement in the reference frame:"
			      "functionality not implemented");
      break ;
    }
  case vpRobot::MIXT_FRAME:
    {
      ERROR_TRACE ("Cannot get a displacement in the mixt frame: "
		   "functionality not implemented");
      throw vpRobotException (vpRobotException::wrongStateError,
			      "Cannot get a displacement in the reference frame:"
			      "functionality not implemented");

      break ;
    }
  }
}

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */

#endif

