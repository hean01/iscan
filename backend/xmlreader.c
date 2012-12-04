/*  xmlreader.c -- 
 *  Copyright (C) 2010  SEIKO EPSON CORPORATION
 *
 *  License: GPLv2+|iscan
 *  Authors: SEIKO EPSON CORPORATION
 *
 *  This file is part of the SANE backend distributed with Image Scan!
 *
 *  Image Scan!'s SANE backend is free software.
 *  You can redistribute it and/or modify it under the terms of the GNU
 *  General Public License as published by the Free Software Foundation;
 *  either version 2 of the License or at your option any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *  You ought to have received a copy of the GNU General Public License
 *  along with this package.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  Linking Image Scan!'s SANE backend statically or dynamically with
 *  other modules is making a combined work based on this SANE backend.
 *  Thus, the terms and conditions of the GNU General Public License
 *  cover the whole combination.
 *
 *  As a special exception, the copyright holders of Image Scan!'s SANE
 *  backend give you permission to link Image Scan!'s SANE backend with
 *  SANE frontends that communicate with Image Scan!'s SANE backend
 *  solely through the SANE Application Programming Interface,
 *  regardless of the license terms of these SANE frontends, and to
 *  copy and distribute the resulting combined work under terms of your
 *  choice, provided that every copy of the combined work is
 *  accompanied by a complete copy of the source code of Image Scan!'s
 *  SANE backend (the version of Image Scan!'s SANE backend used to
 *  produce the combined work), being distributed under the terms of
 *  the GNU General Public License plus this exception.  An independent
 *  module is a module which is not derived from or based on Image
 *  Scan!'s SANE backend.
 *
 *  As a special exception, the copyright holders of Image Scan!'s SANE
 *  backend give you permission to link Image Scan!'s SANE backend with
 *  independent modules that communicate with Image Scan!'s SANE
 *  backend solely through the "Interpreter" interface, regardless of
 *  the license terms of these independent modules, and to copy and
 *  distribute the resulting combined work under terms of your choice,
 *  provided that every copy of the combined work is accompanied by a
 *  complete copy of the source code of Image Scan!'s SANE backend (the
 *  version of Image Scan!'s SANE backend used to produce the combined
 *  work), being distributed under the terms of the GNU General Public
 *  License plus this exception.  An independent module is a module
 *  which is not derived from or based on Image Scan!'s SANE backend.
 *
 *  Note that people who make modified versions of Image Scan!'s SANE
 *  backend are not obligated to grant special exceptions for their
 *  modified versions; it is their choice whether to do so.  The GNU
 *  General Public License gives permission to release a modified
 *  version without this exception; this exception also makes it
 *  possible to release a modified version which carries forward this
 *  exception.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "utils.h"
#include "get-infofile.h"
#include "xmlreader.h"

typedef struct devices{
  char *firmware;
  char *overseas;
  char *japan;
}devices, *devicePtr;

typedef struct commands{
  char *command_type;
  char *command_level;
}commands, *commandPtr;

static void default_profile_set(double color_profile[9]);


static void parseCommand(xmlNodePtr cur, scan_command_t* command);
static unsigned char parseStatus(char *status, char *name);

char *
parseDevices(xmlNodePtr cur, char *name){
  devicePtr ret;

  char *region = NULL;
  char *result = NULL;

  log_call();

  ret = t_calloc (1, devices);
  if(ret == NULL){
    err_major("out of memory");
    return NULL;
  }

  cur = cur->xmlChildrenNode;

  while (cur != NULL) {
    if (!xmlStrcmp(cur->name, (const xmlChar *) "firmware")) {
      if(ret->firmware == NULL && strcmp(name, FIRMWARE) == 0){
	ret->firmware = (char *)xmlGetProp(cur, (const xmlChar *) "name");
	result = strdup(ret->firmware);
	if(!ret->firmware){
	  delete (ret);
	  return NULL;
	}
	break;
      }
    }else if (!xmlStrcmp(cur->name, (const xmlChar *) "model")) {
      region = (char *)xmlGetProp(cur, (const xmlChar *) "region");
      if(region && strcmp(name, MODEL_JAPAN) == 0){
	if(strcmp(name, MODEL_JAPAN) == 0 && strcasecmp(region, "Japan") == 0){
	  ret->japan = (char *)xmlGetProp(cur, (const xmlChar *) "name");
	  result = strdup(ret->japan);
	  delete (ret->japan);
	  if(!result || isspace(result[0]) != 0 || strlen(result) == 0){
	    delete (result);
	    delete (ret);
	    delete (region);
	    result = NULL;
	    err_minor("Model has no Name.");
	  }
	}
	break;
      }else if(!region && strcmp(name, MODEL_OVERSEAS) == 0){
	ret->overseas = (char *)xmlGetProp(cur, (const xmlChar *) "name");
	result = strdup(ret->overseas);
	delete(ret->overseas);
	if(!result || isspace(result[0]) != 0 || strlen(result) == 0){
	  delete (result);
	  delete (ret);
	  delete (region);
	  result = NULL;
	  err_minor("Model has no Name.");
	}
	break;
      }
      delete (region);
    }
    cur = cur->next;
  }

  delete (region);
  delete (ret);

  return result;
}

EpsonScanHard 
parseProfiles(xmlNodePtr cur){
  EpsonScanHard ret;
  int i, j;
  char *tmp;
  char pmat[9][3] = {"rr", "rg", "rb",
                     "gr", "gg", "gb",
                     "br", "bg", "bb"};
  char *profile_type;
  xmlNodePtr curtmp;

  log_call();

  ret = t_calloc (1, EpsonScanHardRec);
  if(ret == NULL){
    err_major("out of memory");
    return NULL;
  }

  /*add default value*/
  for(i = 0; i < 4; i++){
    default_profile_set(ret->color_profile[i]);
  }
  
  cur = curtmp = cur->xmlChildrenNode;
  while(cur != NULL){
    if ((!xmlStrcmp(cur->name, (const xmlChar *) "profile"))){
      curtmp = cur;
      profile_type = (char *)xmlGetProp(cur, (const xmlChar *) "type");
      
      if(strcmp(profile_type, "reflective") == 0){
	i = 0;
      }else if(strcmp(profile_type, "color negative") == 0){
	i = 1;
      }else if(strcmp(profile_type, "monochrome negative") == 0){
	i = 2;
      }else if(strcmp(profile_type, "positive") == 0){
	i = 3;
      }else {
	err_minor("profile of the wrong type.");
	delete (profile_type);
	delete (ret);
	return NULL;
      }
      delete (profile_type);

      j = 0;
      cur = cur->xmlChildrenNode;
      while(cur != NULL){
	if(!xmlStrcmp(cur->name, (const xmlChar *)pmat[j])){
	  tmp = (char*)xmlGetProp(cur, (const xmlChar *) "value");
	  ret->color_profile[i][j] = atof(tmp);
	  delete (tmp);
	  j++;
	}
	cur = cur->next;
      }
      if(j != 9){
	err_minor("Value that is not sufficient exists.");
	default_profile_set(ret->color_profile[i]);
      }
    }
    cur = curtmp;
    cur = curtmp = cur->next;
  }

  return ret;
}

static
void default_profile_set(double color_profile[9]){
  /*default value is assigned*/
  const double default_profile[9] = 
    {1.0000, 0.0000, 0.0000,
     0.0000, 1.0000, 0.0000,
     0.0000, 0.0000, 1.0000} ;
  int j;

  for(j = 0; j < 9; j++){
    color_profile[j] = default_profile[j];
  }
}

scan_command_t *
parseCommands_set(xmlNodePtr cur){
  commandPtr ret;
  scan_command_t* command;

  log_call();

  command = t_calloc (1, scan_command_t);
  if(command == NULL){
    err_major("out of memory");
    return NULL;
  }

  /*default value*/
  command->set_focus_position = 0xFF;
  command->feed = 0xFF;
  command->eject = 0xFF;
  command->lock   = false;
  command->unlock = false;

  ret = t_calloc (1, commands);
  if(ret == NULL){
    err_major("out of memory");
    delete (command);
    return NULL;
  }

  ret->command_type = (char *)xmlGetProp(cur, (const xmlChar *) "type");

  ret->command_level = (char *)xmlGetProp(cur, (const xmlChar *) "level");

  cur = cur->xmlChildrenNode;
  while(cur != NULL){
    if ((!xmlStrcmp(cur->name, (const xmlChar *) "command"))){
      parseCommand(cur, command);
    }
    cur = cur->next;
  }

  delete (ret->command_type);
  delete (ret->command_level);
  delete (ret);
  
  return command;
}

static void
parseCommand(xmlNodePtr cur, scan_command_t* command)
{
  char *tmp;
  char *status = NULL;

  status = (char *)xmlGetProp(cur, (const xmlChar *) "status");

  if((tmp = (char *)xmlGetProp(cur, (const xmlChar *) "name")) != NULL){
    if(strcmp(tmp, "set_focus_position") == 0){
      command->set_focus_position = parseStatus(status, "set_focus_position");
    }else if(strcmp(tmp, "feed") == 0){
      command->feed = parseStatus(status, "feed");
    }else if(strcmp(tmp, "eject") == 0){
      command->eject = parseStatus(status, "eject");
    }else if(strcmp(tmp, "lock") == 0){
      command->lock = true;
      if(status && strcmp(status, "disable") == 0) command->lock = false;
    }else if(strcmp(tmp, "unlock") == 0){
      command->unlock = true;
      if(status && strcmp(status, "disable") == 0) command->unlock = false;
    }
    delete (tmp);
    delete (status);
  }
}

static unsigned char
parseStatus(char *status, char *name)
{
  unsigned char value;

  if(!status || strcmp(status, "enabled") == 0){/*enabled*/
    if(strcmp(name, "set_focus_position") == 0) value = 0x70;
    else if(strcmp(name, "feed") == 0) value = 0x19;
    else if(strcmp(name, "eject") == 0) value = 0x0C;
    else value = 0;
  }else if(strcmp(status, "disabled") == 0){
    value = 0;
  }else{
    value = 0;
  }

  return value;
}

capability_data_t*
parseCapabilities(xmlNodePtr cur)
{
  capability_data_t *capabilities;
  char *tmp;
  char *endp;

  log_call();

  capabilities = t_calloc (1, capability_data_t);
  if(capabilities == NULL){
    err_major("out of memory");
    return NULL;
  }

  cur = cur->xmlChildrenNode;
  while(cur != NULL){
    if ((!xmlStrcmp(cur->name, (const xmlChar *) "scan-area"))){
      endp = tmp = (char *)xmlGetProp(cur, (const xmlChar *) "width");
      capabilities->width = strtol(tmp, &endp, 10);
      if(endp == tmp) capabilities->width = -1;
      if(*endp != '\0')
        err_minor("ignoring trailing garbage (%s)", endp);
      delete (tmp);

      endp = tmp = (char *)xmlGetProp(cur, (const xmlChar *) "height");
      capabilities->height = strtol(tmp, &endp, 10);
      if(endp == tmp) capabilities->height = -1;
      if(*endp != '\0')
        err_minor("ignoring trailing garbage (%s)", endp);
      delete (tmp);

      endp = tmp = (char *)xmlGetProp(cur, (const xmlChar *) "base");
      capabilities->base = strtol(tmp, &endp, 10);
      if(endp == tmp) capabilities->base = 1;
      if(*endp != '\0')
        err_minor("ignoring trailing garbage (%s)", endp);
      delete (tmp);
    }
    cur = cur->next;
  }

  return capabilities;
}
