/*  model-info.c -- per model information objects and cache
 *  Copyright (C) 2010  SEIKO EPSON CORPORATION
 *
 *  License: GPLv2+|iscan
 *  Authors: AVASYS CORPORATION
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "model-info.h"

#include <stdlib.h>
#include <time.h>

#include "list.h"
#include "message.h"
#include "utils.h"

#include "get-infofile.h"
#include "xmlreader.h"


/*! \brief  Points to our model info cache.
 *
 *  Its value is guaranteed to be \c NULL \e outside the scope of a
 *  model_info_cache_init()/model_info_cache_exit() pair.
 */
static       list *_cache   = NULL;


/*! \brief  Directory to search for resource files
 *
 *  The _model_info_ctor() needs this.
 */
static const char *_datadir = NULL;


/*! \brief  Collects all (supported) model information
 */
typedef struct
{
  char *fw_name;                /* key for _model_info_cache_get_info */
  char *overseas;               /* model name */
  char *japan;                  /* model name */
  char *name;                   /* points to one of overseas, japan or
                                 * fw_name, never NULL */
  scan_command_t *command;      /* command customisation info */
  EpsonScanHard   profile;      /* colour profiles */

  bool from_file;               /* origin of our data, used to control
                                 * which resources need to be released
                                 * at destruction time */

  capability_data_t *dfault;
  capability_data_t *adf_duplex;
} _model_info_t;

typedef enum
{
  FEED,
  FOCUS,
  EJECT,

} _command_id_t;

/* Model info cache implementation details */
static _model_info_t * _model_info_cache_get_info (const char *fw_name,
                                                   SANE_Status *status);

/* Model info implementation details */
static _model_info_t * _model_info_ctor (const char *fw_name,
                                         SANE_Status *status);
static void            _model_info_dtor (void *p);
static char *          _model_info_guess_name (const _model_info_t *self);
static SANE_Status     _model_info_merge_file (_model_info_t *self);
static void            _model_info_merge_data (_model_info_t *self,
                                               xmlNodePtr node);

static bool _model_info_set_cmd (const _model_info_t *self,
                                 unsigned char *cmd, _command_id_t id);


/*! \brief  Sets up model info cache support.
 *
 *  Model specific information will be looked for in \a pkgdatadir.
 *
 *  \returns  An opaque pointer to the cache.  An additional \a status
 *            will be returned as well if the argument is not \c NULL.
 */
void *
model_info_cache_init (const char *pkgdatadir, SANE_Status *status)
{
  SANE_Status s = SANE_STATUS_GOOD;

  log_call ("(%s, %p)", pkgdatadir, status);
  require (pkgdatadir);

  if (_cache)
    {
      err_minor ("been here, done that");
      if (0 != strcmp_c (_datadir, pkgdatadir))
        {
          err_major ("already using %s", _datadir);
        }
      if (status) *status = s;
      return _cache;
    }

  _datadir = strdup (pkgdatadir);
  _cache   = list_create ();
  if (!_datadir || !_cache)
    {
      _cache = model_info_cache_exit (_cache);

      s = SANE_STATUS_NO_MEM;
    }

  if (0 != atexit (xmlCleanupParser))
    {
      err_minor ("could not register XML parser cleanup function");
    }

  /* ?FIXME?
   *  Check for existence/readability of _pkgdatadir and log its
   *  absence/presence.  It is _not_ fatal if the directory does
   *  not exist or is not readable.  We just want to note it via
   *  an err_minor().  If not readable, we may want to return a
   *  SANE_STATUS_ACCESS_DENIED.
   */

  if (status) *status = s;
  return _cache;
}


/*! \brief  Tears down model info cache support.
 *
 *  Releases all resources associated with the model info cache.
 *  The \a self argument should be an opaque pointer obtained via a
 *  call to model_info_cache_init().
 *
 *  Note that for error recovery purposes, model_info_cache_init() may
 *  call this function with an empty _cache or no cache at all.
 *
 *  \returns  \c NULL, always
 */
void *
model_info_cache_exit (void *self)
{
  log_call ("(%p)", self);
  require (_cache == self);

  const_delete (_datadir, char *);
  list_destroy (_cache, _model_info_dtor);
  _datadir = NULL;
  _cache   = NULL;

  promise (!_cache);

  return _cache;
}


/*! \brief  Attempts to find model information for \a fw_name.
 *
 *  \return  A pointer to the model information, \c NULL if not found.
 */
const void *
model_info_cache_get_info (const char *fw_name, SANE_Status *status)
{
  /*  Just forward to the internal implementation.  The return
   *  statement will handle the pointer type conversion.
   */
  return _model_info_cache_get_info (fw_name, status);
}


/*! \brief  Returns a best-effort model name based on \a fw_name.
 *
 *  The caller gets to manage the memory occupied by the string that
 *  is returned.  Note that \c NULL may be returned.
 */
char *
model_info_cache_get_model (const char *fw_name)
{
  SANE_Status    s = SANE_STATUS_GOOD;
  _model_info_t *m = NULL;

  log_call ("(%s)", fw_name);
  require (_cache && _datadir);

  if (!fw_name || 0 == strlen (fw_name))
    {
      err_minor ("%s", sane_strstatus (SANE_STATUS_INVAL));
      return strdup ("(unknown model)");
    }

  m = _model_info_cache_get_info (fw_name, &s);
  if (!m)
    {
      err_minor ("%s", sane_strstatus (s));
      return strdup (fw_name);  /* best we can do */
    }

  return strdup (m->name);
}


/*! \brief  Returns a reference to the model name.
 *
 *  Resources associated with the reference are owned by \a self.  The
 *  caller should \e not attempt to release them.
 */
const char *
model_info_get_name (const void *self)
{
  require (self);               /* ?FIXME? check if in _cache? */

  return ((const _model_info_t *) self)->name;
}


/*! \brief  Returns a reference to the model's colour profiles.
 *
 *  Resources associated with the reference are owned by \a self.  The
 *  caller should \e not attempt to release them.
 */
const EpsonScanHard
model_info_get_profile (const void *self)
{
  require (self);               /* ?FIXME? check if in _cache? */

  return ((const _model_info_t *) self)->profile;
}


/*! \brief  Modify selected commands in the \a cmd specification
 *
 *  This function caters to quirks in the command level specification
 *  reported by the device.  Especially commands for hardware options
 *  may be affected.
 *
 *  \return  \c true if commands have been modified, \c false otherwise
 */
bool
model_info_customise_commands (const void *self, EpsonCmd cmd)
{
  bool customised = false;
  _model_info_t *self_ = NULL;

  require (self);               /* ?FIXME? check if in _cache? */

  if (!cmd)
    {
      err_minor ("%s", sane_strstatus (SANE_STATUS_INVAL));
      return customised;
    }

  self_ = (_model_info_t *) self;
  customised |= _model_info_set_cmd (self_, &cmd->set_focus_position, FOCUS);
  customised |= _model_info_set_cmd (self_, &cmd->feed, FEED);
  customised |= _model_info_set_cmd (self_, &cmd->eject, EJECT);

  return customised;
}


/*! \brief  Attempts to find model information for \a fw_name.
 *
 *  Checks for existing information in the cache before it attempts to
 *  add new model information.  Takes care to preserve the cache's cur
 *  member so as not to invalidate existing "iterators".
 *
 *  \return  A pointer to the model information.  \c NULL if not found,
 *           in case anything went wrong trying to add the info to the
 *           cache or the caller passed in garbage.
 */
static _model_info_t *
_model_info_cache_get_info (const char *fw_name, SANE_Status *status)
{
  SANE_Status s = SANE_STATUS_GOOD;
  _model_info_t *info = NULL;
  list_entry *cur = NULL;
  bool found = false;

  log_call ("(%s)", fw_name);
  require (_cache && _datadir);

  if (!fw_name || 0 == strlen (fw_name))
    {
      if (status) *status = SANE_STATUS_INVAL;
      return NULL;
    }

  cur = _cache->cur;            /* check whether cached */
  list_reset (_cache);
  while (!found && (info = list_next (_cache)))
    {
      found = (0 == strcmp_c (info->fw_name, fw_name));
    }
  _cache->cur = cur;

  if (!found)                   /* try to add info to cache */
    {
      info = _model_info_ctor (fw_name, &s);
      if (!info || !list_append (_cache, info))
        {
          _model_info_dtor (info);
          info = NULL;
        }
    }

  if (status) *status = s;
  return info;
}


/*! \brief  Creates and initialises a model info object.
 *
 *  \return  A valid model info object or \c NULL if unable to acquire
 *           the necessary memory resources.
 */
static _model_info_t *
_model_info_ctor (const char *fw_name, SANE_Status *status)
{
  SANE_Status s = SANE_STATUS_GOOD;
  _model_info_t *self = NULL;

  log_call ("(%s)", fw_name);
  require (fw_name);

  self = t_calloc (1, _model_info_t);
  if (!self)
    {
      if (status) *status = SANE_STATUS_NO_MEM;
      return NULL;
    }

  self->fw_name = strdup (fw_name);
  if (!self->fw_name)
    {
      _model_info_dtor (self);
      if (status) *status = SANE_STATUS_NO_MEM;
      return NULL;
    }

  /*  Set defaults using data defined in the source code.  The various
   *  getters decide a decent default in case self->fw_name is not one
   *  of the names for which we have data in our sources.
   */
  self->overseas = get_scanner_data (self->fw_name, MODEL_OVERSEAS);
  self->japan    = get_scanner_data (self->fw_name, MODEL_JAPAN);
  self->profile  = get_epson_scan_hard (self->fw_name);
  self->command  = get_scan_command (self->fw_name);

  self->from_file = false;

  s = _model_info_merge_file (self);

  self->name = _model_info_guess_name (self);

  if (self)                     /* make sure things are compliant */
    {
      promise (self->fw_name && self->name);
      promise (   self->name == self->fw_name
               || self->name == self->overseas
               || self->name == self->japan);
      promise (self->profile);
      promise (self->command);
    }
  if (status) *status = s;
  return self;
}


/*! \brief  Destroys a model object.
 *
 *  Releases any resources acquired throughout the object's life time.
 */
static void
_model_info_dtor (void *self)
{
  _model_info_t *p = (_model_info_t *) self;

  if (!p) return;

  if (p->from_file)
    {
      /* :FIXME: p->profile may have been acquired as several
       *         individual arrays at construction.  Check!
       */
      if (p->profile != get_epson_scan_hard (p->fw_name))
        delete (p->profile);
      if (p->command != get_scan_command (p->fw_name))
        delete (p->command);
      delete (p->overseas);
      delete (p->japan);
      if(p->dfault)
        {
          delete (p->dfault->option);
          delete (p->dfault->mode);
        }
      delete (p->dfault);
      if (p->adf_duplex)
        {
          delete (p->adf_duplex->option);
          delete (p->adf_duplex->mode);
        }
      delete (p->adf_duplex);
    }

  delete (p->fw_name);
  delete (p);
}

/*! \brief  Returns a best effort guess for model name on the device
 *
 *  This functions implements the policy used to decide which of the
 *  various model names \c self->name should point to.  The function
 *  is intended for use at construction of \a self.
 */
static char *
_model_info_guess_name (const _model_info_t *self)
{
  require (self);

  if (self->japan && self->overseas)
    {
      time_t     lt  = time (NULL);
      struct tm *ptr = localtime (&lt);

      if (ptr && 0 == strncmp_c ("JST", ptr->tm_zone, 3))
        {
          return self->japan;
        }
      else
        {
          return self->overseas;
        }
    }

  if (self->japan) return self->japan;
  if (self->overseas) return self->overseas;
  return self->fw_name;
}

static char *
get_path_name (const char *hex_name, char *path_name, size_t path_size)
{
  char  *path;
  size_t size = snprintf (path_name, path_size, "%s%s%s%s",
                          _datadir, FILE_SEP_STR, hex_name,
                          ".xml");

  if (size > -1 || size < path_size)
    return path_name;

  if (size > -1)
    path_size = size + 1;
  else
    path_size *= 2;

  if (0 == path_size)
    path_size = 512;

  path = t_realloc (path_name, path_size, char);
  if (!path)
    {
      delete (path_name);
      return NULL;
    }
  path_name = path;

  return get_path_name (hex_name, path_name, path_size);
}

/*! \brief  Attempts to merge model information from a data file.
 */
static SANE_Status
_model_info_merge_file (_model_info_t *self)
{
  xmlDocPtr doc = NULL;

  char  *path_name = NULL;
  char  *hex_name  = NULL;

  require (self);

  hex_name = fw_name_to_hex (self->fw_name);

  if (!hex_name) return SANE_STATUS_NO_MEM;

  path_name = get_path_name (hex_name, NULL, 0);
  delete (hex_name);            /* no longer needed */

  if (!path_name) return SANE_STATUS_NO_MEM;

  log_data ("%s", path_name);

  doc = xmlReadFile (path_name, NULL,
                     XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
  delete (path_name);           /* no longer needed */

  if (doc)
    {
      _model_info_merge_data (self, xmlDocGetRootElement (doc));
      xmlFreeDoc (doc);
    }
  else
    {
      xmlErrorPtr p = xmlGetLastError ();
      if (p) err_minor ("%s", p->message);
    }

  return SANE_STATUS_GOOD;
}

static void
_model_info_merge_data (_model_info_t *self, xmlNodePtr node)
{
  /*  Define the XML element tags that we are able to merge.
   */
  const xmlChar *device   = (xmlChar *) "device";
  const xmlChar *profiles = (xmlChar *) "profile-set";
  const xmlChar *commands = (xmlChar *) "command-set";
  const xmlChar *capabilities = (xmlChar *) "capabilities";
  const xmlChar *cap_option = (xmlChar *) "option";
  const xmlChar *cap_mode = (xmlChar *) "mode";
  char *tmp;

  require (self);

  if (!node) return;

  node = node->xmlChildrenNode;
  while (node)
  {
    if (!xmlIsBlankNode (node))
    {
      if (0 == xmlStrcmp (node->name, device))
      {
      char *tmp = NULL;

      tmp = parseDevices (node, MODEL_OVERSEAS);
      if (tmp) self->overseas = tmp;

      tmp = parseDevices (node, MODEL_JAPAN);
      if (tmp) self->japan = tmp;
      }
      else if (0 == xmlStrcmp (node->name, profiles))
      {
        EpsonScanHard profile = parseProfiles (node);
        if (profile) self->profile = profile;
      }
      else if (0 == xmlStrcmp (node->name, commands))
      {
        scan_command_t *command = parseCommands_set (node);
        if (command) self->command = command;
      }
      else if (0 == xmlStrcmp (node->name, capabilities))
      {
        tmp = (char *)xmlGetProp(node, (const xmlChar *) cap_mode);
        if(strcmp_c(tmp, "duplex") == 0){
          capability_data_t *capability = parseCapabilities (node);
          if (capability) {
            self->adf_duplex = capability;
            self->adf_duplex->option = (char *)xmlGetProp(node, (const xmlChar *) cap_option);
            self->adf_duplex->mode = tmp;
          }
        }else {
          capability_data_t *capability = parseCapabilities (node);
          if (capability) {
            self->dfault = capability;
            self->dfault->option = (char *)xmlGetProp(node, (const xmlChar *) cap_option);
            self->dfault->mode = tmp;
          }
        }
      }
    }
    node = node->next;
  }
  self->from_file = true;
}

/*! \brief  Customises a single command
 *
 *  \return  \c true if \a cmd was modified, \c false otherwise
 */
static bool
_model_info_set_cmd (const _model_info_t *self, unsigned char *cmd,
                     _command_id_t id)
{
  unsigned char cmd_ = ILLEGAL_CMD;

  require (self && cmd);

  if (FEED  == id) cmd_ = self->command->feed;
  if (FOCUS == id) cmd_ = self->command->set_focus_position;
  if (EJECT == id) cmd_ = self->command->eject;

  if (ILLEGAL_CMD != cmd_)
    {
      *cmd = cmd_;
      return true;
    }
  return false;
}

bool
model_info_has_lock_commands (const void *self)
{
  _model_info_t *self_ = NULL;

  require (self);

  self_ = (_model_info_t *) self;

  return (self_->command->lock && self_->command->unlock);
}

scan_area_t 
model_info_max_scan_area(const void *self, const char *option, const char *mode)
{
  _model_info_t *self_ = NULL;
  scan_area_t scan_area;
  
  require (self);
  require (option);

  scan_area.width = SANE_FIX(-1);
  scan_area.height = SANE_FIX(-1);

  self_ = (_model_info_t *) self;

  if(strcmp(option, "adf") == 0 && strcmp_c(mode, "duplex") == 0){
    if(self_->adf_duplex){
      scan_area.width = SANE_FIX (self_->adf_duplex->width * MM_PER_INCH / self_->adf_duplex->base);
      scan_area.height = SANE_FIX (self_->adf_duplex->height * MM_PER_INCH / self_->adf_duplex->base);
    }
  }else {
    if(self_->dfault){
      scan_area.width = SANE_FIX (self_->dfault->width * MM_PER_INCH / self_->dfault->base);
      scan_area.height = SANE_FIX (self_->dfault->height * MM_PER_INCH / self_->dfault->base);
    }
  }

  return scan_area;
}
