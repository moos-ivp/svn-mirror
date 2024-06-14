/*****************************************************************/
/*    NAME: Michael Benjamin                                     */
/*    ORGN: Dept of Mechanical Engineering, MIT, Cambridge MA    */
/*    FILE: PickPos.cpp                                          */
/*    DATE: Sep 22nd, 2018                                       */
/*                                                               */
/* This file is part of MOOS-IvP                                 */
/*                                                               */
/* MOOS-IvP is free software: you can redistribute it and/or     */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation, either version  */
/* 3 of the License, or (at your option) any later version.      */
/*                                                               */
/* MOOS-IvP is distributed in the hope that it will be useful,   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty   */
/* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See  */
/* the GNU General Public License for more details.              */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with MOOS-IvP.  If not, see                     */
/* <http://www.gnu.org/licenses/>.                               */
/*****************************************************************/

#include <iostream>
#include <cstdlib>
#include <cmath>
#include "PickPos.h"
#include "VQuals.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include "GeomUtils.h"
#include "FileBuffer.h"
#include "XYFormatUtilsPoly.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

PickPos::PickPos()
{
  // By default, positions and headings are rounded to nearest integer
  m_pt_snap   = 0.1;
  m_hdg_snap   = 1;
  m_spd_snap   = 0.1;

  // When making random pts from poly(s), min buffer dist between pts
  m_buffer_dist = 10;

  // When making random pts, num tries to find pt satisfying buffer
  m_max_tries = 1000;

  // If multiline true, output each point on a separate line
  m_multiline = true;

  // Hdg_val config params to support picking position hdg vals
  m_hdg_type = "none";
  m_hdg_val1 = 0;
  m_hdg_val2 = 0;
  m_hdg_val3 = 0;

  // Spd_val config params to support picking position speed vals
  m_spd_type = "none";
  m_spd_val1 = 0;
  m_spd_val2 = 0;

  // For Picking points on a circle
  m_circ_x = 0;
  m_circ_y = 0;
  m_circ_rad = 0;
  m_circ_set = false;
  
  m_pick_amt = 10;

  m_vnames  = false;
  m_colors  = false;
  m_indices = false;
  m_verbose = false;
  m_vname_start_ix = 0;
  
  m_reverse_names = false;
  
  m_grp_type = "random";
  m_output_type = "full";

  m_headers_enabled = false;

  m_global_nearest = -1;
  m_reuse = false;
  
  setVNameCacheOne();
  setColorCache();
}


//---------------------------------------------------------
// Procedure: setPickAmt()

bool PickPos::setPickAmt(string str)
{
  if(!isNumber(str))
    return(false);

  int ival = atoi(str.c_str());
  if(ival <= 0)
    return(false);

  m_pick_amt = (unsigned int)(ival);
  return(true);
}

//---------------------------------------------------------
// Procedure: setVNameStartIX()

bool PickPos::setVNameStartIX(string str)
{
  if(!isNumber(str))
    return(false);

  int ival = atoi(str.c_str());
  if(ival < 0)
    return(false);

  m_vname_start_ix = (unsigned int)(ival);
  return(true);
}

//---------------------------------------------------------
// Procedure: setBufferDist()

bool PickPos::setBufferDist(string str)
{
  return(setNonNegDoubleOnString(m_buffer_dist, str));
}

//---------------------------------------------------------
// Procedure: setMaxTries()

bool PickPos::setMaxTries(string str)
{
  if(!isNumber(str))
    return(false);

  int ival = atoi(str.c_str());
  if(ival <= 0)
    return(false);

  m_max_tries = (unsigned int)(ival);
  return(true); 
}

//---------------------------------------------------------
// Procedure: setHeadingSnap()

bool PickPos::setHeadingSnap(string str)
{
  return(setNonNegDoubleOnString(m_hdg_snap, str));
}

//---------------------------------------------------------
// Procedure: setSpeedSnap()

bool PickPos::setSpeedSnap(string str)
{
  return(setNonNegDoubleOnString(m_spd_snap, str));
}

//---------------------------------------------------------
// Procedure: setPointSnap()

bool PickPos::setPointSnap(string str)
{
  return(setNonNegDoubleOnString(m_pt_snap, str));
}

//---------------------------------------------------------
// Procedure: setOutputType

bool PickPos::setOutputType(string str)
{
  if((str == "full") || (str == "terse")) {
    m_output_type = str;
    return(true);
  }
  return(false);
}

//---------------------------------------------------------
// Procedure: setVNames()

bool PickPos::setVNames()
{
  m_vnames = true;
  return(true);
}

//---------------------------------------------------------
// Procedure: setVNames(string)

bool PickPos::setVNames(string str)
{
  m_vnames = true;

  if(str == "herons")
    str = "abe,ben,cal,deb,eve,fin,max,ned,oak,pal";
  
  vector<string> names = parseString(str, ',');
  for(unsigned int i=0; i<names.size(); i++) {
    string name = stripBlankEnds(names[i]);
    if(strContainsWhite(name))
      return(false);
    if(strContains(name, '*'))
      return(false);
    if(strContains(name, '"'))
      return(false);
    if(strContains(name, '\"'))
      return(false);
    if(strContains(name, ')'))
      return(false);
    if(strContains(name, '('))
      return(false);
    if(strContains(name, '$'))
      return(false);
  }

  for(unsigned int i=0; i<names.size(); i++) {
    string name = stripBlankEnds(names[i]);
    if(i < m_vname_cache.size())
      m_vname_cache[i] = name;
  }
    
  return(true);
}

//---------------------------------------------------------
// Procedure: setColors()

bool PickPos::setColors(string str)
{
  m_colors = true;
  
  vector<string> colors = parseString(str, ',');
  for(unsigned int i=0; i<colors.size(); i++) {
    string color = stripBlankEnds(colors[i]);
    if(!isColor(color))
      return(false);
  }

  for(unsigned int i=0; i<colors.size(); i++) {
    string color = stripBlankEnds(colors[i]);
    if(i < m_color_cache.size())
      m_color_cache[i] = color;
  }
    
  return(true);
}


//---------------------------------------------------------
// Procedure: setHdgConfig
//     Notes: "none" --> no heading value assigned to each pt
//            "-45:45" --> rand hdg value in [-45:45] chosen
//            "12,20,relbng" --> hdg value relbng pt 12,20 chosen

bool PickPos::setHdgConfig(string str)
{
  if(isNumber(str)) {
    double val = atof(str.c_str());
    if((val < -180) || (val >= 360))
      return(false);
    m_hdg_type = "rand";
    m_hdg_val1 = val;
    m_hdg_val2 = val;
    return(true);
  }
  
  if(strContains(str, ':')) {
    vector<string> svector = parseString(str, ':');
    if(svector.size() != 2)
      return(false);
    if(!isNumber(svector[0]) || !isNumber(svector[1]))
      return(false);
    double low = atof(svector[0].c_str());
    double hgh = atof(svector[1].c_str());
    if((low < -180) || (low >= 360))
      return(false);
    if((hgh < -180) || (hgh >= 360))
      return(false);
    if(low > hgh)
      return(false);
    
    m_hdg_type = "rand";
    m_hdg_val1 = low;
    m_hdg_val2 = hgh;
    return(true);
  }
    
  if(strContains(str, ',')) {
    vector<string> svector = parseString(str, ',');
    if(svector.size() != 3)
      return(false);
    if(!isNumber(svector[0]) || !isNumber(svector[1]) ||
       !isNumber(svector[2]))
      return(false);
    double xpos = atof(svector[0].c_str());
    double ypos = atof(svector[1].c_str());
    double rbng = atof(svector[2].c_str());
    if((rbng < -180) || (rbng >= 360))
      return(false);
    
    m_hdg_type = "rbng";
    m_hdg_val1 = xpos;
    m_hdg_val2 = ypos;
    m_hdg_val3 = rbng;
    return(true);
  }
    
  return(true); 
}


//---------------------------------------------------------
// Procedure: setSpdConfig
//     Notes: "none" --> no speed value assigned to each pt
//            "1:5"  --> rand spd value in [1:5] chosen

bool PickPos::setSpdConfig(string str)
{
  if(isNumber(str)) {
    double val = atof(str.c_str());
    if(val < 0)
      return(false);
    m_spd_type = "rand";
    m_spd_val1 = val;
    m_spd_val2 = val;
    return(true);
  }
  
  if(strContains(str, ':')) {
    vector<string> svector = parseString(str, ':');
    if(svector.size() != 2)
      return(false);
    if(!isNumber(svector[0]) || !isNumber(svector[1]))
      return(false);
    double low = atof(svector[0].c_str());
    double hgh = atof(svector[1].c_str());
    if(low < 0)
      return(false);
    if(hgh < 0)
      return(false);
    if(low > hgh)
      return(false);
    
    m_spd_type = "rand";
    m_spd_val1 = low;
    m_spd_val2 = hgh;
    return(true);
  }
    
  return(true); 
}


//---------------------------------------------------------
// Procedure: setGroups
//   Example: "red,blue"
//            "jays,dogs,cats,cats"
//            "red,blue:alt"
//            "red,blue,red,red:alt"

bool PickPos::setGroups(string str)
{
  if(strEnds(str, ":alt")) {
    rbiteString(str, ':');
    m_grp_type = "alternating";
  }
  
  vector<string> svector = parseString(str, ',');
  for(unsigned int i=0; i<svector.size(); i++) {
    string group = svector[i];
    m_groups.push_back(group);
  }
    
  return(true); 
}


//---------------------------------------------------------
// Procedure: setResultFile()

bool PickPos::setResultFile(string str)
{
  if(!okFileToWrite(str))
    return(false);

  m_result_file = str;
  return(true);
}

//---------------------------------------------------------
// Procedure: setCircle()
//   Example: "x=23,y=43,rad=100"

bool PickPos::setCircle(string str)
{
  if(isQuoted(str))
    str = stripQuotes(str);
  
  double x = 0;
  double y = 0;;
  double rad = 0;
  bool x_set = false;
  bool y_set = false;
  bool rad_set = false;
  
  vector<string> svector = parseString(str, ',');
  for(unsigned i=0; i<svector.size(); i++) {
    string param = biteStringX(svector[i], '=');
    string value = svector[i];
    if((param == "x") && isNumber(value)) {
      x = atof(value.c_str());
      x_set = true;
    }
    else if((param == "y") && isNumber(value)) {
      y = atof(value.c_str());
      y_set = true;
    }
    else if((param == "rad") && isNumber(value)) {
      rad = atof(value.c_str());
      rad_set = true;
    }
    else
      return(false);
  }
  
  if(!x_set || !y_set || !rad_set)
    return(false);

  m_circ_x = x;
  m_circ_y = y;
  m_circ_rad = rad;
  m_circ_set = true;
  return(true);
}

//---------------------------------------------------------
// Procedure: addPosFile()
//   Example: x=23, y=34, hdg=90
//            x=-23, y=94

bool PickPos::addPosFile(string filename)
{
  if(!okFileToRead(filename))
    return(false);

  vector<string> lines = fileBuffer(filename);
  for(unsigned int i=0; i<lines.size(); i++) {
    vector<string> pairs = parseString(lines[i], ',');

    bool ok_line = true;
    bool x_set = false;
    bool y_set = false;

    for(unsigned int j=0; j<pairs.size(); j++) {
      string param = biteStringX(pairs[j], '=');
      string value = pairs[j];
      if(param == "x") {
	if(isNumber(value))
	  x_set = true;
      }
      else if(param == "y") {
	if(isNumber(value))
	  y_set = true;
      }
      else if(param == "hdg") {
	if(!isNumber(value))
	  ok_line = false;
      }
      else
	ok_line = false;
    }
    if(ok_line && x_set && y_set)
      m_file_positions.push_back(lines[i]);
  }

  return(true);
}


//---------------------------------------------------------
// Procedure: pick()

bool PickPos::pick()
{
  if(m_verbose)
    cout << "Generating " << m_pick_amt << " positions" << endl;

  if(m_file_positions.size() != 0)    
    pickPosByFile();
  else if(m_fld_generator.polygonCount() > 0)
    pickPosByPoly();
  else if(m_circ_set)
    pickPosByCircle();

  if(m_hdg_type != "none")
    pickHeadingVals();

  if(m_spd_type != "none")
    pickSpeedVals();

  if(m_vname_cache.size() != 0)
    pickVehicleNames();
  
  if(m_colors)
    pickColors();

  if(m_indices)
    pickIndices();

  if(m_groups.size() != 0)
    pickGroupNames();

  if(m_headers_enabled) {
    cout << "# Values chosen by the pickpos utility" << endl;
    cout << "# " << m_arg_summary << endl;
  }

  if(m_verbose) {
    if(m_hdg_type != "none") {
      cout << "heading type: " << m_hdg_type << endl;
      cout << "  heading val1: " << m_hdg_val1 << endl;
      cout << "  heading val2: " << m_hdg_val2 << endl;
      cout << "  heading val3: " << m_hdg_val3 << endl;
    }
    if(m_spd_type != "none") {
      cout << "speed type: " << m_spd_type << endl;
      cout << "  speed val1: " << m_spd_val1 << endl;
      cout << "  speed val2: " << m_spd_val2 << endl;
    }
  }
  printChoices();
  
  return(true);
}


//---------------------------------------------------------
// Procedure: pickPosByFile()

void PickPos::pickPosByFile()
{
  unsigned int choices = m_file_positions.size();
  
  if(m_pick_amt > choices) {
    cout << "Cannot pick " << m_pick_amt << " positions." << endl;
    cout << "File(s) only had " << choices << " lines." << endl;
    exit(1);
  }
  srand(time(NULL));

  // Create an array of Booleans the same size as num of file choices
  vector<bool> bool_positions(choices, false);
   
  for(unsigned int i=0; i<m_pick_amt; i++) {
    int rval = rand() % ((int)(choices));

    bool found = false;
    while(!found) {
      for(unsigned int i=(unsigned int)(rval); ((i<choices) && !found); i++) {
	if(bool_positions[i] == false) {
	  bool_positions[i] = true;
	  found = true;
	  m_pick_positions.push_back(m_file_positions[i]);
	}
      }
      rval = 0;
    }
  }
}

//---------------------------------------------------------
// Procedure: pickPosByPoly()

void PickPos::pickPosByPoly()
{
  if(m_fld_generator.size() == 0) {
    cout << "Cannot pick " << m_pick_amt << " positions." << endl;
    cout << "No polygons have been specified." << endl;
    exit(1);
  }
  srand(time(NULL));

  m_fld_generator.setSnap(m_pt_snap);
  m_fld_generator.setBufferDist(m_buffer_dist);
  m_fld_generator.setFlexBuffer(true);
  m_fld_generator.generatePoints(m_pick_amt);

  vector<XYPoint> points = m_fld_generator.getPoints();
  if(points.size() != m_pick_amt) {
    cout << "Unable to squeeze " << m_pick_amt << " pts in given region." << endl;
    cout << "Wanted: " << m_pick_amt << ", Found: " << points.size() << endl;
    exit(2);
  }
  
  for(unsigned int i=0; i<points.size(); i++) {
    XYPoint pick_pt = points[i];
    
    double x = pick_pt.get_vx();
    double y = pick_pt.get_vy();
    string s="x="+doubleToStringX(x,2)+",y="+doubleToStringX(y,2);
    
    m_pick_positions.push_back(s);
    m_near_positions.push_back(0);
  }    

  m_near_positions = m_fld_generator.getNearestVals();
  m_global_nearest = m_fld_generator.getGlobalNearest();
}

//---------------------------------------------------------
// Procedure: pickPosByCircle()
//      Note: The min_sep value, if < 0 indicates that all the points
//            should be evenly distributed. Otherwise, a positive
//            value indicates that the arc length between points
//            should be at least as large as minsep.
//   Returns: false if the min_sep criteria cannot be upheld
//            true otherwise

bool PickPos::pickPosByCircle(double min_sep)
{
  if(m_pick_amt == 0)
    return(false);

  double m_circ_x = 10;
  double m_circ_y = 20;
  double m_circ_r = 50;
  
  // First handle case where all points are evenly separated
  if(min_sep < 0) {
    double angle_delta = 360.0 / (double)(m_pick_amt);

    // First pick an arbitrary initial angle    
    double ang1 = ((double)(rand() % 3600)) / 100.0;
    
    vector<XYPoint> points;
    for(unsigned int i=0; i<m_pick_amt; i++) {
      double ang = angle360(ang1 + (i*angle_delta));

      double newx,newy;
      projectPoint(ang, m_circ_r, m_circ_x, m_circ_y, newx, newy);

      string s="x="+doubleToStringX(newx,2)+",y="+doubleToStringX(newy,2);

      m_pick_positions.push_back(s);
      m_near_positions.push_back(0);
    }
  }
  return(true);
}

//---------------------------------------------------------
// Procedure: pickHeadingVals()

void PickPos::pickHeadingVals()
{
  // Part 1: Handle simple case where the user does not want headings
  if(m_hdg_type == "none")
    return;
  
  srand(time(NULL));

  // Sanity check
  if((m_pick_positions.size() != m_pick_amt) && (m_hdg_type == "rbng")) {
    cout << "The rel_bng heading pick mode can only be use while also";
    cout << "picking positions. Exiting now." << endl;
    exit(1);
  }
  
  // Part 2: Handle making random headings from a range of headings
  if(m_hdg_type == "rand") {
    
    double range = m_hdg_val2 - m_hdg_val1;
    for(unsigned int i=0; i<m_pick_amt; i++) {
      if(range == 0)
	m_pick_headings.push_back(m_hdg_val1);
      else {
	int choices = (int)((100 * range));
	int rval = rand() % choices;
	double hdg = m_hdg_val1 + (double)(rval) / 100;
	m_pick_headings.push_back(hdg);
      }
    }
  }

  // Part 3: Handle making random headings relative to a point
  if(m_hdg_type == "rbng") {
    for(unsigned int i=0; i<m_pick_amt; i++) {
      string xstr = tokStringParse(m_pick_positions[i], "x", ',', '=');
      string ystr = tokStringParse(m_pick_positions[i], "y", ',', '=');

      double hdg = 0;
      if((xstr != "") && (ystr != "")) {
	double x = atof(xstr.c_str());
	double y = atof(ystr.c_str());
	double ang = relAng(x, y, m_hdg_val1, m_hdg_val2);
	
	hdg = angle180(ang - m_hdg_val3);
      }
      m_pick_headings.push_back(hdg);
    }
  }
}

//---------------------------------------------------------
// Procedure: pickSpeedVals()

void PickPos::pickSpeedVals()
{
  // Part 1: Handle simple case where the user does not want speeds
  if(m_spd_type == "none")
    return;
  
  srand(time(NULL));

  // Part 2: Handle making random speeds from a range of speeds
  if(m_spd_type == "rand") {
    double range = m_spd_val2 - m_spd_val1;
    for(unsigned int i=0; i<m_pick_amt; i++) {
      if(range == 0) {
	m_pick_speeds.push_back(m_spd_val1);
      }
      else {
	int choices = (int)((10000 * range));
	int rval = rand() % choices;
	double spd = m_spd_val1 + (double)(rval) / 10000;
	m_pick_speeds.push_back(spd);
      }
    }
  }
}

//---------------------------------------------------------
// Procedure: pickGroupNames()

void PickPos::pickGroupNames()
{
  // Part 1: Handle simple case where user does not want groups
  int choices = (int)(m_groups.size());
  if(choices == 0)
    return;
  
  srand(time(NULL));

  // Part 2: Select random group names from configured set
  if(m_grp_type == "random") {
    for(unsigned int i=0; i<m_pick_amt; i++) {
      int rval = rand() % choices;
      string chosen_group = m_groups[rval];
      m_pick_groups.push_back(chosen_group);
    }
  }
  // Part 3: Select alternating group names from configured set
  if(m_grp_type == "alternating") {
    for(unsigned int i=0; i<m_pick_amt; i++) {
      int alt_val = i % choices;
      string chosen_group = m_groups[alt_val];
      m_pick_groups.push_back(chosen_group);
    }
  }
}

//---------------------------------------------------------
// Procedure: setVNameCacheOne()

bool PickPos::setVNameCacheOne()
{
  m_vname_cache.clear();
  for(unsigned int i=0; i<52; i++)
    m_vname_cache.push_back(getIndexVName1(i));
  return(true);
}

//---------------------------------------------------------
// Procedure: setVNameCacheTwo()

bool PickPos::setVNameCacheTwo()
{
  m_vname_cache.clear();
  for(unsigned int i=0; i<52; i++)
    m_vname_cache.push_back(getIndexVName2(i));
  return(true);
}

//---------------------------------------------------------
// Procedure: setVNameCacheThree()

bool PickPos::setVNameCacheThree()
{
  m_vname_cache.clear();
  for(unsigned int i=0; i<52; i++)
    m_vname_cache.push_back(getIndexVName3(i));
  return(true);
}

//---------------------------------------------------------
// Procedure: setVNameCacheFour()

bool PickPos::setVNameCacheFour()
{
  m_vname_cache.clear();
  for(unsigned int i=0; i<52; i++)
    m_vname_cache.push_back(getIndexVName4(i));
  return(true);
}

//---------------------------------------------------------
// Procedure: pickVehicleNames()

void PickPos::pickVehicleNames()
{
  int choices = (int)(m_vname_cache.size());
  if(choices == 0)
    return;

  unsigned int vix = m_vname_start_ix;
  
  for(unsigned int i=vix; i<m_pick_amt+vix; i++) {
    int index = i % choices;
    if(m_reverse_names)
      index = (choices-1)-i;

    m_pick_vnames.push_back(m_vname_cache[index]);
  }
}

//---------------------------------------------------------
// Procedure: pickIndices()

void PickPos::pickIndices()
{
  unsigned int vix = m_vname_start_ix;
  
  for(unsigned int i=vix; i<m_pick_amt+vix; i++) 
    m_pick_indices.push_back(i);
}

//---------------------------------------------------------
// Procedure: setColorCache()

void PickPos::setColorCache()
{
  m_color_cache.clear();
  for(unsigned int i=0; i<30; i++)
    m_color_cache.push_back(getIndexVColor(i));
}

//---------------------------------------------------------
// Procedure: pickColors()

void PickPos::pickColors()
{
  int choices = (int)(m_color_cache.size());
  if(choices == 0)
    return;
  
  for(unsigned int i=0; i<m_pick_amt; i++) {
    int index = i % choices;
    m_pick_colors.push_back(m_color_cache[index]);
  }
}

//---------------------------------------------------------
// Procedure: printChoices()

void PickPos::printChoices()
{
  if(okFileToRead(m_result_file) && m_reuse)
    return;

  FILE *fptr = 0;
  if(m_result_file != "") {
    fptr = fopen(m_result_file.c_str(), "w");
    if(!fptr) {
      cout << "Unable to open file: " << m_result_file << endl;
      return;
    }
  }

  
  if(m_pick_indices.size() > 0) {
    for(unsigned int i=0; i<m_pick_indices.size(); i++)
      cout << m_pick_indices[i] << endl;
    return;
  }

  // All three vectors will be of size either N or 0.
  // Find out what N is here and set to max_index
  unsigned int max_index = m_pick_positions.size();
  if(m_pick_headings.size() > max_index)
    max_index = m_pick_headings.size();
  if(m_pick_speeds.size() > max_index)
    max_index = m_pick_speeds.size();
  if(m_pick_vnames.size() > max_index)
    max_index = m_pick_vnames.size();
  if(m_pick_groups.size() > max_index)
    max_index = m_pick_groups.size();

  for(unsigned int i=0; i<max_index; i++) {
    string line;
    bool position_info = false;
    if(i<m_pick_positions.size()) {
      line = m_pick_positions[i];
      position_info = true;
    }

    if((m_hdg_type != "none") && (i<m_pick_headings.size())) {
      double hdg = angle360(m_pick_headings[i]);
      hdg = snapToStep(hdg, m_hdg_snap);
      if(line != "")
	line += ",";
      if(position_info)
	line += "heading=";
      line += doubleToStringX(hdg,3);
    }

    if((m_spd_type != "none") && (i<m_pick_speeds.size())) {
      double spd = m_pick_speeds[i];
      spd = snapToStep(spd, m_spd_snap);
      if(line != "")
	line += ",";
      if(position_info)
	line += "speed=";
      line += doubleToStringX(spd,5);
    }
      
    if(m_vnames && (i<m_pick_vnames.size())) {
      if(line != "")
	line += ",";
      line += m_pick_vnames[i];
    }
      
    if(m_colors && (i<m_pick_colors.size())) {
      if(line != "")
	line += ",";
      if(position_info)
	line += "color=";
      line += m_pick_colors[i];
    }
      
    if((m_groups.size() > 0) && (i<m_pick_groups.size())) {
      if(line != "")
	line += ",";
      line += m_pick_groups[i];
    }
      
    if(m_output_type == "terse") {
      line = findReplace(line, "x=", "");
      line = findReplace(line, "y=", "");
      line = findReplace(line, "heading=", "");
      line = findReplace(line, "speed=", "");
      line = findReplace(line, "color=", "");
    }

    if(fptr != 0) {
      fprintf(fptr, "%s\n", line.c_str());
      continue;
    }

    cout << line;

    if(m_verbose) {
      if(i < m_near_positions.size())
	cout << "   nearest=" << doubleToString(m_near_positions[i],2);
    }
    cout << endl;
  }
  if(m_verbose)
    cout << "Global nearest = " << doubleToStringX(m_global_nearest,2) << endl;
}


