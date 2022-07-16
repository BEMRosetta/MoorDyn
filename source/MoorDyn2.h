/*
 * Copyright (c) 2014 Matt Hall <mtjhall@alumni.uvic.ca>
 *
 * This file is part of MoorDyn.  MoorDyn is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * MoorDyn is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MoorDyn.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __MOORDYN2_H__
#define __MOORDYN2_H__

#include "MoorDynAPI.h"
#include "Waves.h"
#include "Connection.h"
#include "Line.h"
#include "Body.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef WIN32
#include <windows.h>
#endif

	/** @defgroup new_c_api The C API
	 *  @{
	 */

	/// A mooring system instance
	typedef struct __MoorDyn* MoorDyn;

	/** @brief Creates a MoorDyn instance
	 *
	 * At the time of creating a new MoorDyn instance, the input file is read
	 * and all the objects and structures are created. You must call afterwards
	 * MoorDyn_Init() to compute the initial conditions
	 *
	 * @param infilename The input file, if either NULL or "", then
	 * "Mooring/lines.txt" will be considered
	 * @return The mooring instance, NULL if errors happened
	 */
	MoorDyn DECLDIR MoorDyn_Create(const char* infilename);

	/** @brief Get the number of coupled Degrees Of Freedom (DOFs)
	 *
	 * The number of components for some parameters in MoorDyn_Init() and
	 * MoorDyn_Step() can be known using this function
	 * @return The number of coupled DOF, 0 if errors are detected
	 */
	unsigned int DECLDIR MoorDyn_NCoupledDOF(MoorDyn system);

	/** @brief Set the instance verbosity level
	 * @param system The Moordyn system
	 * @param verbosity The verbosity level. It can take the following values
	 *  - MOORDYN_DBG_LEVEL Every single message will be printed
	 *  - MOORDYN_MSG_LEVEL Messages specially designed to help debugging the
	 * code will be omitted
	 *  - MOORDYN_WRN_LEVEL Just errors and warnings will be reported
	 *  - MOORDYN_ERR_LEVEL Just errors will be reported
	 *  - MOORDYN_NO_OUTPUT No info will be reported
	 * @return 0 If the verbosity level is correctly set, an error code
	 * otherwise (see @ref moordyn_errors)
	 */
	int DECLDIR MoorDyn_SetVerbosity(MoorDyn system, int verbosity);

	/** @brief Set the instance log file
	 * @param system The Moordyn system
	 * @param log_path The file path to print the log file
	 * @return 0 If the log file is correctly set, an error code otherwise
	 * (see @ref moordyn_errors)
	 */
	int DECLDIR MoorDyn_SetLogFile(MoorDyn system, const char* log_path);

	/** @brief Set the instance log file printing level
	 * @param system The Moordyn system
	 * @param verbosity The log file print level. It can take the following
	 * values
	 *  - MOORDYN_DBG_LEVEL Every single message will be printed
	 *  - MOORDYN_MSG_LEVEL Messages specially designed to help debugging the
	 * code will be omitted
	 *  - MOORDYN_WRN_LEVEL Just errors and warnings will be reported
	 *  - MOORDYN_ERR_LEVEL Just errors will be reported
	 *  - MOORDYN_NO_OUTPUT No info will be reported
	 * @return 0 If the printing level is correctly set, an error code otherwise
	 * (see @ref moordyn_errors)
	 */
	int DECLDIR MoorDyn_SetLogLevel(MoorDyn system, int verbosity);

	/** @brief Log a message
	 * @param system The Moordyn system
	 * @param level The message level. It can take the following values
	 *  - MOORDYN_DBG_LEVEL for debugging messages
	 *  - MOORDYN_MSG_LEVEL for regular information messages
	 *  - MOORDYN_WRN_LEVEL for warnings
	 *  - MOORDYN_ERR_LEVEL for errors
	 * @param msg The message to log
	 * @return 0 If the printing level is correctly set, an error code otherwise
	 * (see @ref moordyn_errors)
	 * @note This messages are subjected to the same rules than the inner
	 * messages, i.e. if @p level is lower than the threshold levels set with
	 * MoorDyn_SetVerbosity() and MoorDyn_SetLogLevel(), the message will not be
	 * logged in the terminal and the log file respectively
	 * @note This function will not log the file, line and function where it is
	 * called from, not even in case of warnings or errors
	 */
	int DECLDIR MoorDyn_Log(MoorDyn system, int level, const char* msg);

	/** @brief Compute the initial condition of a MoorDyn system
	 *
	 * At the time of creating a new MoorDyn instance, the input file is read
	 * and all the objects and structures are created. You must call afterwards
	 * MoorDyn_Init() to compute the initial conditions
	 *
	 * @param system The Moordyn system
	 * @param x Position vector (6 components per coupled body or cantilevered
	 * rod and 3 components per pinned rod or coupled connection)
	 * @param xd Velocity vector (6 components per coupled body or cantilevered
	 * rod and 3 components per pinned rod or coupled connection)
	 * @return 0 If the mooring system is correctly initialized, an error code
	 * otherwise (see @ref moordyn_errors)
	 * @note MoorDyn_NCoupledDOF() can be used to know the number of components
	 * required for \p x and \p xd
	 */
	int DECLDIR MoorDyn_Init(MoorDyn system, const double* x, const double* xd);

	/** @brief Runs a time step of the MoorDyn system
	 * @param system The Moordyn system
	 * @param x Position vector
	 * @param xd Velocity vector
	 * @param f Output forces
	 * @return 0 if the mooring system has correctly evolved, an error code
	 * otherwise (see @ref moordyn_errors)
	 * @note MoorDyn_NCoupledDOF() can be used to know the number of components
	 * required for \p x, \p xd and \p f
	 */
	int DECLDIR MoorDyn_Step(MoorDyn system,
	                         const double* x,
	                         const double* xd,
	                         double* f,
	                         double* t,
	                         double* dt);

	/** @brief Releases MoorDyn allocated resources
	 * @param system The Moordyn system
	 * @return 0 If the mooring system is correctly destroyed, an error code
	 * otherwise (see @ref moordyn_errors)
	 */
	int DECLDIR MoorDyn_Close(MoorDyn system);

	/** @brief Get the wave kinematics instance
	 *
	 * The wave kinematics instance is only useful if WaveKin option is set to 2
	 * in the input file.
	 * @param system The Moordyn system
	 * @return The waves instance, NULL if errors happened
	 */
	MoorDynWaves DECLDIR MoorDyn_GetWaves(MoorDyn system);

	/** @brief Initializes the external Wave kinetics
	 *
	 * This is useless unless WaveKin option is set to 1 in the input file. If
	 * that is the case, remember calling this function after MoorDyn_Init()
	 * @param system The Moordyn system
	 * @param n The number of points where the wave kinematics shall be provided
	 * @return 0 If the mooring system is correctly destroyed, an error code
	 * otherwise (see @ref moordyn_errors)
	 */
	int DECLDIR MoorDyn_ExternalWaveKinInit(MoorDyn system, unsigned int* n);

	/** @brief Get the points where the waves kinematics shall be provided
	 *
	 * The kinematics on those points shall be provided just if WaveKin is set
	 * to 1 in the input file
	 * @param system The Moordyn system
	 * @param r The output coordinates (3 components per point)
	 * @return 0 If the data is correctly set, an error code otherwise
	 * (see @ref moordyn_errors)
	 * @see MoorDyn_ExternalWaveKinInit()
	 */
	int DECLDIR MoorDyn_GetWaveKinCoordinates(MoorDyn system, double* r);

	/** @brief Set the kinematics of the waves
	 *
	 * Use this function if WaveKin option is set to 1 in the input file
	 * @param system The Moordyn system
	 * @param U The velocities at the points (3 components per point)
	 * @param Ud The accelerations at the points (3 components per point)
	 * @return 0 If the data is correctly set, an error code otherwise
	 * (see @ref moordyn_errors)
	 * @see MoorDyn_ExternalWaveKinInit()
	 * @see MoorDyn_GetWaveKinCoordinates()
	 */
	int DECLDIR MoorDyn_SetWaveKin(MoorDyn system,
	                               const double* U,
	                               const double* Ud,
	                               double t);

	/** @brief Get the number of bodies
	 *
	 * Remember that the first body index is 1
	 * @param system The Moordyn system
	 * @return The number of bodies, which may be 0 if errors happened
	 * (see @ref moordyn_errors)
	 */
	unsigned int DECLDIR MoorDyn_GetNumberBodies(MoorDyn system);

	/** @brief Get the number of rods
	 *
	 * Remember that the first rod index is 1
	 * @param system The Moordyn system
	 * @return The number of rods, which may be 0 if errors happened
	 * (see @ref moordyn_errors)
	 */
	unsigned int DECLDIR MoorDyn_GetNumberRods(MoorDyn system);

	/** @brief Get the number of connections
	 *
	 * Remember that the first connection index is 1
	 * @param system The Moordyn system
	 * @return The number of connections, which may be 0 if errors happened
	 * (see @ref moordyn_errors)
	 */
	unsigned int DECLDIR MoorDyn_GetNumberConnections(MoorDyn system);

	/** @brief Get a connection
	 * @param system The Moordyn system
	 * @param c The connection
	 * @return The connection instance, NULL if errors happened
	 */
	MoorDynConnection DECLDIR MoorDyn_GetConnection(MoorDyn system,
	                                                unsigned int c);

	/** @brief Get the number of lines
	 *
	 * Remember that the first line index is 1
	 * @param system The Moordyn system
	 * @return The number of lines, which may be 0 if errors happened
	 * (see @ref moordyn_errors)
	 */
	unsigned int DECLDIR MoorDyn_GetNumberLines(MoorDyn system);

	/** @brief Get a line instance
	 * @param system The Moordyn system
	 * @param l The line identifier (from 1 to the number of lines)
	 * @return The line instance, NULL if errors happened
	 */
	MoorDynLine DECLDIR MoorDyn_GetLine(MoorDyn system, unsigned int l);

	/** @brief Function for providing FASTv7 customary line tension quantities
	 * @param system The Moordyn system
	 * @param numLines The number of lines
	 * @param FairHTen Allocated memory for the \p numLines horizontal forces at
	 * the fairlead
	 * @param FairVTen Allocated memory for the \p numLines vertical forces at
	 * the fairlead
	 * @param AnchHTen Allocated memory for the \p numLines horizontal forces at
	 * the anchor
	 * @param AnchVTen Allocated memory for the \p numLines vertical forces at
	 * the anchor
	 * @return 0 If the data is correctly set, an error code otherwise
	 * (see @ref moordyn_errors)
	 */
	int DECLDIR MoorDyn_GetFASTtens(MoorDyn system,
	                                const int* numLines,
	                                float FairHTen[],
	                                float FairVTen[],
	                                float AnchHTen[],
	                                float AnchVTen[]);

	/** @brief Draw the lines and connections in the active OpenGL context
	 *
	 * The OpenGL context is assumed to be created by the caller before calling
	 * this function
	 * @param system The Moordyn system
	 * @return 0 If the data is correctly set, an error code otherwise
	 * (see @ref moordyn_errors)
	 */
	int DECLDIR MoorDyn_DrawWithGL(MoorDyn system);

	/**
	 * @}
	 */

#ifdef __cplusplus
}
#endif

#endif
