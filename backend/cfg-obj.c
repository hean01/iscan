/*  cfg-obj.c -- configuration objects
 *  Copyright (C) 2008, 2009  SEIKO EPSON CORPORATION
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


/*! \file
 *  \todo Split the probing responsibility off into the respective
 *  channel classes.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "cfg-obj.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>

#include "command.h"
#include "hw-data.h"
#include "net-obj.h"
#include "model-info.h"
#include "ipc.h"
#include "utils.h"

/* Deprecated includes */
#include "include/sane/sanei_usb.h"
#include "include/sane/sanei_scsi.h"


#ifndef SANE_CONFIG_DIR_NAME
#define SANE_CONFIG_DIR_NAME "/etc/sane.d"
#endif

#define DEV_NAME_SEP_STR ":"
#define DEV_NAME_SEP DEV_NAME_SEP_STR[0]


const char *cfg_file_name = "epkowa.conf";


typedef struct
{
  bool active[_CFG_KEY_ID_TERMINATOR_];
  list * seen[_CFG_KEY_ID_TERMINATOR_];

} cfg_type;

static cfg_type *_cfg = NULL;

static cfg_key_type _cfg_key[] =
  {
    "net",
    "pio",
    "scsi",
    "usb",

    "interpreter",
    "fs-blacklist",

    "option",
  };

static char* _opt_vals[] =
  {
    "prefer-adf",
  };


static FILE *_cfg_fopen_conf (const char *filename);
static FILE *_cfg_fopen_data (const char *dirname, const char *key);


static cfg_key_id_type _cfg_getline (char **line, size_t *size, FILE *fp);


static bool _cfg_is_bare_key (const char *string);
static bool _cfg_is_valid_net_entry (const char *string);
static bool _cfg_is_valid_scsi_entry (const char *string);
static bool _cfg_is_valid_usb_entry (const char *string);
static bool _cfg_is_valid_interpreter_entry (const char *string);
static bool _cfg_is_valid_fs_blacklist_entry (const char *string);
static bool _cfg_is_valid_option_entry (const char *string);

typedef bool (*validator) (const char *);

static validator _cfg_validate[] =
  {
    _cfg_is_valid_net_entry,
    _cfg_is_bare_key,
    _cfg_is_valid_scsi_entry,
    _cfg_is_valid_usb_entry,

    _cfg_is_valid_interpreter_entry,
    _cfg_is_valid_fs_blacklist_entry,

    _cfg_is_valid_option_entry,
  };


static bool _cfg_register_no_op (const char *string);
static bool _cfg_register_net_entry (const char *string);
static bool _cfg_register_scsi_entry (const char *string);
static bool _cfg_register_usb_entry (const char *string);
static bool _cfg_register_interpreter_entry (const char *string);
static bool _cfg_register_fs_blacklist_entry (const char *string);
static bool _cfg_register_option_entry (const char *string);

typedef bool (*registrar) (const char *);

static registrar _cfg_register[] =
  {
    _cfg_register_net_entry,
    _cfg_register_no_op,
    _cfg_register_scsi_entry,
    _cfg_register_usb_entry,

    _cfg_register_interpreter_entry,
    _cfg_register_fs_blacklist_entry,

    _cfg_register_option_entry,
  };


static void _cfg_probe_no_op (list *dev_list);
static void _cfg_probe_net (list *dev_list);
static void _cfg_probe_scsi (list *dev_list);
static void _cfg_probe_usb (list *dev_list);
static void _cfg_probe_interpreter (list *dev_list);

typedef void (*probe) (list *);

static probe _cfg_probe[] =
  {
    _cfg_probe_net,
    _cfg_probe_no_op,
    _cfg_probe_scsi,
    _cfg_probe_usb,

    _cfg_probe_interpreter,
    _cfg_probe_no_op,

    _cfg_probe_no_op,
  };


static SANE_Status _cfg_attach (SANE_String_Const dev_name, list *dev_list);
static SANE_String_Const _cfg_get_vendor (SANE_String_Const dev_name);
static SANE_String_Const _cfg_get_model (SANE_String_Const dev_name);
static SANE_String_Const _cfg_get_type (SANE_String_Const dev_name);

static bool _cfg_have_interpreter (const char *library, const char *firmware);

/*  Helper stuff to hack around sanei_usb/sanei_scsi API requirements
    used in the various _cfg_probe_*() implementations.
 */
static       list *_cfg_dev_list = NULL;
static const char *_cfg_dev_key  = NULL;
static SANE_Status _cfg_scsi_attach (SANE_String_Const dev_name);
static SANE_Status _cfg_usb_attach (SANE_String_Const dev_name);


static void _cfg_net_dtor (void *);
static void _cfg_scsi_dtor (void *);
static void _cfg_interpreter_dtor (void *);

typedef void (*destructor) (void *);

static destructor _cfg_dtor[] =
  {
    _cfg_net_dtor,
    free,
    _cfg_scsi_dtor,
    free,

    _cfg_interpreter_dtor,
    free,

    free,
  };


/*! Creates and initialises a configuration object.

    The configuration object is initialised with per key data files in
    the \a pkgdatadir and a configuration file, \c cfg_file_name.  The
    configuration file is searched for in list of directories with the
    first file found being used.

    The list of directories can be customised via a \c SANE_CONFIG_DIR
    environment variable.  This variable uses the exact same syntax as
    the \c PATH environment variable with this difference that a final
    colon, \c :, appends the default locations.

    When no \c SANE_CONFIG_DIR variable is set, the default locations
    will be used.  The default locations are, in order:
    - the process' current working directory
    - the system's SANE configuration directory

    On most system's the above is equivalent to setting:
    \code
    SANE_CONFIG_DIR=.:/etc/sane.d
    \endcode
 */
void *
cfg_init (const char *pkgdatadir, SANE_Status *status)
{
  SANE_Status s = SANE_STATUS_GOOD;

  char *lc_ctype = NULL;
  FILE *fp = NULL;
  cfg_key_id_type id;

  log_call ("(%s, %p)", pkgdatadir, status);
  require (num_of (_cfg_key) == _CFG_KEY_ID_TERMINATOR_);

  if (_cfg)
    {
      if (pkgdatadir) err_minor ("been here, done that");
      if (status) *status = s;
      return _cfg;
    }

  _cfg = t_calloc (1, cfg_type);
  if (!_cfg)
    {
      if (status) *status = SANE_STATUS_NO_MEM;
      return _cfg;
    }

  lc_ctype = setlocale (LC_CTYPE, "C");

  if (pkgdatadir)
    {
      /* Look for data files containing information about the
       * devices we "officially" support.  Keywords without a
       * corresponding data file are not an error.
       */
      id = 0;
      while (num_of (_cfg_key) > id)
        {
          fp = _cfg_fopen_data (pkgdatadir, _cfg_key[id]);
          if (fp)
            {
              size_t size = 0;
              char  *line = NULL;

              while (_cfg_getline (&line, &size, fp) == id)
                {
                  log_info ("line: '%s'", line);

                  if (0 != strcmp_c (line, _cfg_key[id]))
                    {
                      _cfg_register[id] (line);
                    }
                }
              delete (line);
              if (fclose (fp))
                {
                  err_minor ("%s%s%s: %s",
                             pkgdatadir, FILE_SEP_STR, _cfg_key[id],
                             strerror (errno));
                }
            }
          ++id;
        }

      /* Allow for interpreter based packages to (un)register
       * themselves upon (un)installation in a suitable place
       * so we don't have to track them anymore.
       */
      id = CFG_KEY_INTERPRETER;
      fp = _cfg_fopen_data (PKGLOCALSTATEDIR, _cfg_key[id]);
      if (fp)
        {
          size_t size = 0;
          char  *line = NULL;

          while (_cfg_getline (&line, &size, fp) == id)
            {
              log_info ("line: '%s'", line);

              if (0 != strcmp_c (line, _cfg_key[id]))
                {
                  _cfg_register[id] (line);
                }
            }
          delete (line);
          if (fclose (fp))
            {
              err_minor ("%s%s%s: %s",
                         PKGLOCALSTATEDIR, FILE_SEP_STR, _cfg_key[id],
                         strerror (errno));
            }
        }
    }

  fp = _cfg_fopen_conf (cfg_file_name);
  if (fp)
    {
      size_t size = 0;
      char  *line = NULL;

      while (num_of (_cfg_key) != (id = _cfg_getline (&line, &size, fp)))
        {
          log_info ("line: '%s'", line);

          _cfg->active[id] = true;

          if (0 != strcmp_c (line, _cfg_key[id]))
            {
              _cfg_register[id] (line);
            }
        }
      delete (line);
      if (fclose (fp))
        {
          err_minor ("%s: %s", cfg_file_name, strerror (errno));
        }
    }

  setlocale (LC_CTYPE, lc_ctype);

  /* For backwards compatibity with older configuration files.
   */
  if (cfg_has (_cfg, CFG_KEY_USB)
      && 0 < list_size (_cfg->seen[CFG_KEY_INTERPRETER]))
    {
      log_info ("enabling interpreter support");
      cfg_set (_cfg, CFG_KEY_INTERPRETER, true);
    }

  if (status) *status = s;
  return _cfg;
}

/*! Releases resources acquired by a configuration object.

    Always returns \c NULL.

    \sa cfg_init
 */
void *
cfg_exit (void *self)
{
  log_call ("(%p)", self);
  require (_cfg == self);

  if (_cfg)
    {
      int id;

      for (id = 0; num_of (_cfg_key) > id; ++id)
        {
          if (_cfg->seen[id])
            {
              list_destroy (_cfg->seen[id], _cfg_dtor[id]);
              _cfg->seen[id] = NULL;
            }
        }

      delete (_cfg);
    }
  return _cfg;
}


/*! Searches for supported devices and adds them to a \a dev_list.
 */
void
cfg_find (const void *self, cfg_key_id_type id, list *dev_list)
{
  log_call ("(%p, %u, %p)", self, id, dev_list);
  require (_cfg && _cfg == self);
  require (0 <= id && id < num_of (_cfg_key));

  if (!cfg_has (self, id)) return;

  _cfg_probe[id] (dev_list);
}

/*! Provides read-only access to the supported devices found.

    \bug  Access is not read-only.
 */
list *
cfg_seen (const void *self, cfg_key_id_type id)
{
  log_call ("(%p, %u)", self, id);
  require (_cfg && _cfg == self);
  require (0 <= id && id < num_of (_cfg_key));

  return _cfg->seen[id];
}

/*! Tells whether configuration for a key \a id is in effect.

    \sa cfg_set
 */
bool
cfg_has (const void *self, cfg_key_id_type id)
{
  log_call ("(%p, %u)", self, id);
  require (_cfg && _cfg == self);
  require (0 <= id && id < num_of (_cfg_key));

  return _cfg->active[id];
}

/*! Enables or disabled support for a configuration key.

    This can be used to disable configured functionality when the
    required components are not available.  Think network support
    and interpreter based scanners.
 */
void
cfg_set (void *self, cfg_key_id_type id, bool value)
{
  log_call ("(%p, %u, %u)", self, id, value);
  require (_cfg && _cfg == self);
  require (0 <= id && id < num_of (_cfg_key));

  _cfg->active[id] = value;
}


bool
cfg_has_value (const void *self, cfg_key_id_type id, const char* val)
{
  list *seen = cfg_seen (self, id);
  const char *found = NULL;

  if (val && seen)
    {
      list_entry *cur = seen->cur;
      list_reset (seen);

      while ((found = list_next (seen))
             && 0 != strcmp_c (val, found))
        {
          /* condition does all the processing */
        }
      seen->cur = cur;
    }
  log_info ("check for %s in %s: %s", val, _cfg_key[id],
            (NULL != found) ? "found" : "not found");
  return (NULL != found);
}


/*! Returns the string that corresponds to the key \a id.
 */
cfg_key_type
cfg_key (const void *self, cfg_key_id_type id)
{
  log_call ("(%p, %u)", self, id);
  require (_cfg && _cfg == self);
  require (0 <= id && id < num_of (_cfg_key));

  return _cfg_key[id];
}


/*! Tries to find and open a configuration file.

    Returns a valid file pointer if successfull, \c NULL otherwise.

    \sa cfg_init
 */
static FILE *
_cfg_fopen_conf (const char *name)
{
  const char *default_path = "." PATH_SEP_STR SANE_CONFIG_DIR_NAME;

  char *path = getenv ("SANE_CONFIG_DIR");
  char *next;
  char *dir;
  FILE *fp;

  log_call ("(%s)", name);
  require (name);

  if (path)
    {
      size_t len = strlen (path);
      char *p;

      if (0 < len && PATH_SEP == path[len-1])
        {
          len += strlen (default_path);
        }

      p = t_malloc (len + 1, char);
      if (p)
        {
          strcpy (p, path);
          if (strlen (path) < len)
            {
              strcat (p, default_path);
            }
          path = strdup (p);
          delete (p);
        }
      else
        {
          err_major ("SANE_CONFIG_DIR: %s", strerror (ENOMEM));
        }
    }
  else
    {
      path = strdup (default_path);
    }

  if (!path)
    {
      errno = ENOMEM;
      return NULL;
    }

  fp   = NULL;
  next = path;

  while (!fp && (dir = strsep (&next, PATH_SEP_STR)))
    {
      fp = _cfg_fopen_data (dir, name);
    }
  delete (path);

  return fp;
}

/*! Tries to open a configuration or data file.

    Returns a valid file pointer if successfull, \c NULL otherwise.

    \sa cfg_init
 */
static FILE *
_cfg_fopen_data (const char *dir, const char *name)
{
  FILE *fp = NULL;
  char  file[PATH_MAX];
  int   n = snprintf (file, sizeof (file), "%s%c%s", dir, FILE_SEP, name);

  log_call ("(%s, %s)", dir, name);
  require (dir && name);

  if (sizeof (file) > n)
    {
      fp = fopen (file, "rb");
      if (!fp)
        {
          log_info ("%s: %s", file, strerror (errno));
        }
    }
  else
    {
      err_minor ("%s%c%s: %s",
                 dir, FILE_SEP, name, strerror (ENAMETOOLONG));
    }
  if (fp) log_info ("using '%s'", file);

  return fp;
}

/*! Returns a validated line of a configuration or data file.

    Validated lines are guaranteed to start with one of the entries in
    \c _cfg_key and adhere to the syntax for that particular key.

    Comments and empty lines will be silently ignored, invalid entries
    will be logged and skipped and leading and/or trailing white space
    as well as trailing comments will be stripped.

    Comments start with a sharp, \c #.  An escape mechanism is \e not
    supported.

    Returns \c _CFG_KEY_ID_TERMINATOR_ if no validated line can be
    found.
 */
static cfg_key_id_type
_cfg_getline (char **line, size_t *size, FILE *fp)
{
  int id = num_of (_cfg_key);
  bool valid = false;
  ssize_t n;

  char *lc_ctype;

  require (line && size && fp);

  lc_ctype = setlocale (LC_CTYPE, "C");

  while (!valid && -1 != (n = getline (line, size, fp)))
    {
      char *s;

      log_data ("looking at '%s'", *line);

      s = strchr (*line, '#');
      if (s) *s = '\0';                 /* chomp trailing comments */

      s = *line;
      n = strlen (s);

      while (0 < n && (isspace (*s)))   /* whitespace removal */
        --n, ++s;
      while (0 < n && (isspace (s[n-1])))
        --n, s[n] = '\0';

      log_data ("payload is '%s'", s);

      require (strlen (s) == n);

      if (0 < n)                        /* content validation */
        {
          id = 0;
          while (num_of (_cfg_key) > id
                 && 0 != strncmp_c (s, _cfg_key[id], strlen (_cfg_key[id])))
            {
              log_data ("%s !~ %s", _cfg_key[id], s);
              ++id;
            }

          valid = (num_of (_cfg_key) > id) && _cfg_validate[id] (s);
          if (valid && s != *line)
            {
              memmove (*line, s, strlen (s) + 1);
            }
          if (!valid)
            {
              err_major ("invalid: '%s'", s);
            }
        }
    }

  setlocale (LC_CTYPE, lc_ctype);

  return (-1 == n ? num_of (_cfg_key) : id);
}

/*! Tells whether a \a string consists of a single key.
 */
static bool
_cfg_is_bare_key (const char *string)
{
  int id = 0;

  require (string);

  while (num_of (_cfg_key) > id
         && 0 != strcmp_c (string, _cfg_key[id]))
    {
      ++id;
    }
  return (num_of (_cfg_key) > id);
}

/*! Says whether a \a string makes up a valid network device entry.
 *
 *  Valid network device entries have one of the following formats:
 *  - \c net IPv4-address[( |:)port]
 *  - \c net hostname[( |:)port]
 *
 *  \todo Tighten up validation!
 *  \todo Consider support for IPv4-address ranges by use of netmasks
 *        or CIDR notation.
 */
static bool
_cfg_is_valid_net_entry (const char *string)
{
  const char *p = string;

  int  port = 0;
  char c = '\0';

  require (string);

  if (0 == strcmp_c (string, _cfg_key[CFG_KEY_NET]))
    return false;

  if (0 != strncmp_c (string, _cfg_key[CFG_KEY_NET],
                      strlen (_cfg_key[CFG_KEY_NET]))
      || !isspace (string[strlen(_cfg_key[CFG_KEY_NET])]))
    return false;

  p += strlen (_cfg_key[CFG_KEY_NET]);
  while (*p && isspace (*p))
    {
      ++p;
    }

  if (1 == sscanf (p, "%*s %d%1s", &port, &c)) return true;

  // check for [ip/host]:[port] format
  while (*p && !isspace (*p) && ':' != *p)
    {
      ++p;
    }
  if ('\0' == *p) return true; // case without a port number
  if (isspace (*p)) return false;
  if (1 == sscanf (p, ":%d%1s", &port, &c)) return true;

  return false;
}

/*! Says whether a \a string is a valid SCSI device entry.
 *
 *  Valid SCSI device entries have the following format:
 *  - \c scsi[ vendor[ model]]
 *
 *  Vendor and model specifications are case-insensitive.
 *
 *  \todo Consider support for absolute path names to a device.
 */
static bool
_cfg_is_valid_scsi_entry (const char *string)
{
  const char *p = string;

  require (string);

  if (0 == strcmp_c (string, _cfg_key[CFG_KEY_SCSI]))
    return true;

  if (0 != strncmp_c (string, _cfg_key[CFG_KEY_SCSI],
                      strlen (_cfg_key[CFG_KEY_SCSI]))
      || !isspace (string[strlen(_cfg_key[CFG_KEY_SCSI])]))
    return false;

  p += strlen (_cfg_key[CFG_KEY_SCSI]);
  while (*p && isspace (*p))
    {
      ++p;
    }
  while (*p && !isspace (*p))
    {
      ++p;
    }
  if (!*p) return true;         /* vendor only */

  while (*p && isspace (*p))
    {
      ++p;
    }
  while (*p && !isspace (*p))
    {
      ++p;
    }
  if (!*p) return true;         /* vendor model specification */

  return false;                 /* we have trailing cruft */
}

/*! Decides whether a \a string constitutes a valid USB entry.
 *
 *  Valid USB entries have the following format:
 *  - \c usb[ vendor_id prodoct_id]
 *
 *  Both IDs, if present, shall be \c 0x or \c 0X prefixed sequences
 *  of one to four hexadecimal digits.  Case of the digits \c A thru
 *  \c F is irrelevant and may be mixed, even within a single ID.
 *
 *  \bug Does not catch IDs without intervening whitespace.
 */
static bool
_cfg_is_valid_usb_entry (const char *string)
{
  unsigned int vendor;
  unsigned int product;
  char x[] = "x";               /* matches 'x' or 'X' */
  char c = '\0';                /* matches trailing content */

  require (string);

  if (0 == strcmp_c (string, _cfg_key[CFG_KEY_USB]))
    return true;

  if (0 != strncmp_c (string, _cfg_key[CFG_KEY_USB],
                      strlen (_cfg_key[CFG_KEY_USB]))
      || !isspace (string[strlen(_cfg_key[CFG_KEY_USB])]))
    return false;

  return (4 == sscanf (string, "%*s 0%1[xX]%4x 0%1[xX]%4x%1s",
                       x, &vendor, x, &product, &c));
}

/*! Decides whether a \a string constitutes a valid interpreter entry.
 *
 *  Valid interpreter entries have the following format:
 *  - \c interpreter usb vendor_id prodoct_id library[ firmware-file]
 *
 *  Both IDs, if present, shall be \c 0x or \c 0X prefixed sequences
 *  of one to four hexadecimal digits.  Case of the digits \c A thru
 *  \c F is irrelevant and may be mixed, even within a single ID.
 *
 *  \bug Does not catch IDs without intervening whitespace.
 */
static bool
_cfg_is_valid_interpreter_entry (const char *string)
{
  const char *p1 = string;
  const char *p2 = _cfg_key[CFG_KEY_INTERPRETER];

  unsigned int vendor;
  unsigned int product;
  char x[] = "x";               /* matches 'x' or 'X' */
  char c1 = '\0';               /* matches required whitespace */
  char c2 = '\0';               /* matches start of library-name */

  require (string);

  if (0 != strncmp_c (p1, p2, strlen (p2)))
    return false;

  p1 += strlen (p2);
  while (*p1 && isspace (*p1))
    {
      ++p1;
    }

  p2 = _cfg_key[CFG_KEY_USB];
  if (0 != strncmp_c (p1, p2, strlen (p2))
      || !isspace (p1[strlen(p2)]))
    return false;

  return (6 == sscanf (string, "%*s %*s 0%1[xX]%4x 0%1[xX]%4x%c %c",
                       x, &vendor, x, &product, &c1, &c2)
          && isspace (c1));
}

/*! Says whether \a string is a valid generic key/value entry.
 *
 *  Valid entries have the following format:
 *  - \c <key> <value>
 */
static bool
_cfg_is_valid_key_value_entry (cfg_key_id_type key_id, const char *string)
{
  require (string);

  if (0 != strncmp_c (string, _cfg_key[key_id],
                      strlen (_cfg_key[key_id]))
      || !isspace (string[strlen(_cfg_key[key_id])]))
    return false;

  return true;
}

/*! Says whether \a string is a valid FS blacklist entry.
 *
 *  Valid blacklist entries have the following format:
 *  - \c fs-blacklist fw_name
 */
static bool
_cfg_is_valid_fs_blacklist_entry (const char *string)
{
  return _cfg_is_valid_key_value_entry (CFG_KEY_FS_BLACKLIST, string);
}

/*! Says whether \a string is a valid option entry.
 *
 *  Valid option entries have the following format:
 *  - \c option option_name
 */
static bool
_cfg_is_valid_option_entry (const char *string)
{
  return _cfg_is_valid_key_value_entry (CFG_KEY_OPTION, string);
}

/*! Does \e not register a \a string at all.

    This function should never be called at run-time.  It is solely
    meant as a place holder for use in the \c _cfg_register array.

    Always returns \c false.
 */
static bool
_cfg_register_no_op (const char *string)
{
  require (string);

  err_minor ("internal error: '%s'", string);
  return false;
}

static bool
_cfg_register_net_entry (const char *string)
{
  cfg_net_info *info = NULL;

  require (string);

  if (!_cfg->seen[CFG_KEY_NET])
    _cfg->seen[CFG_KEY_NET] = list_create ();
  if (!_cfg->seen[CFG_KEY_NET]) return false;

  info = t_malloc (1, cfg_net_info);
  if (info)
    {
      char *spec = NULL;
      int   port = 0;

      string += strlen (_cfg_key[CFG_KEY_NET]); 
      while (++string && isspace (*string)) 
        { 
          /* condition does all the processing */ 
        } 
      spec = strdup (string);

      if (1 == sscanf (spec, "%*s %d", &port))
        {
          char *colon;
          char *p = spec;
          while (*p && !isspace (*p))
            {
              ++p;
            }
          *p = ':';
          colon = p;
          ++p;
          while (*p && isspace (*p))
            {
              ++p;
            }
          memmove (colon + 1, p, strlen (p) + 1);
        }

      if (list_append (_cfg->seen[CFG_KEY_NET], info))
        {
          info->spec = spec;
          log_info ("registered '%s'", info->spec);
        }
      else
        {
          delete (spec);
          delete (info);
        }
    }

  return (NULL != info);
}

/*! Attemps to register supported SCSI device information.
 */
static bool
_cfg_register_scsi_entry (const char *string)
{
  cfg_scsi_info *info = NULL;

  require (string);

  if (!_cfg->seen[CFG_KEY_SCSI])
    _cfg->seen[CFG_KEY_SCSI] = list_create ();
  if (!_cfg->seen[CFG_KEY_SCSI]) return false;

  info = t_malloc (1, cfg_scsi_info);
  if (info)
    {
      char *vendor = NULL;
      char *model  = NULL;

      sscanf (string, "%*s %as %as", &vendor, &model);

      if (list_append (_cfg->seen[CFG_KEY_SCSI], info))
        {
          info->vendor = vendor;
          info->model  = model;
          log_info ("registered '%s'", string);
        }
      else
        {
          delete (vendor);
          delete (model);
          delete (info);
        }
    }

  return (NULL != info);
}

/*! Attempts to register information for a USB entry.
 *
 *  Relies on the assumption that the \a string has been validated and
 *  returns \c true if successfull.  When not successful, this will be
 *  caused by lack of memory and the function returns \c false.
 */
static bool
_cfg_register_usb_entry (const char *string)
{
  cfg_usb_info *info = NULL;

  require (string);

  if (!_cfg->seen[CFG_KEY_USB])
    _cfg->seen[CFG_KEY_USB] = list_create ();
  if (!_cfg->seen[CFG_KEY_USB]) return false;

  info = t_malloc (1, cfg_usb_info);
  if (info)
    {
      unsigned int vendor;
      unsigned int product;

      sscanf (string, "%*s %x %x", &vendor, &product);

      if (list_append (_cfg->seen[CFG_KEY_USB], info))
        {
          info->vendor  = vendor;
          info->product = product;
          log_info ("registered '%s'", string);
        }
      else
        {
          delete (info);
        }
    }

  return (NULL != info);
}

/*! Tries to register information for an interpreter entry.
 *
 *  \todo Check for existence of the \a library and \a firmware file.
 */
static bool
_cfg_register_interpreter_entry (const char *string)
{
  cfg_interpreter_info *info = NULL;

  require (string);

  if (!_cfg->seen[CFG_KEY_INTERPRETER])
    _cfg->seen[CFG_KEY_INTERPRETER] = list_create ();
  if (!_cfg->seen[CFG_KEY_INTERPRETER]) return false;

  info = t_malloc (1, cfg_interpreter_info);
  if (info)
    {
      unsigned int vendor;
      unsigned int product;
      char *library  = NULL;
      char *firmware = NULL;

      sscanf (string, "%*s %*s %x %x %as %as",
              &vendor, &product, &library, &firmware);

      if (library && _cfg_have_interpreter (library, firmware)
          && list_append (_cfg->seen[CFG_KEY_INTERPRETER], info))
        {
          info->vendor   = vendor;
          info->product  = product;
          info->library  = library;
          info->firmware = firmware;
          log_info ("registered '%s'", string);
        }
      else
        {
          delete (library);
          delete (firmware);
          delete (info);
        }
    }
  return (NULL != info);
}

/*! Registers information for a generic key/value entry
 */
static bool
_cfg_register_key_value_entry (cfg_key_id_type key_id, const char *string)
{
  char *value = NULL;

  require (string);

  if (!_cfg->seen[key_id])
    _cfg->seen[key_id] = list_create ();
  if (!_cfg->seen[key_id]) return false;

  string += strlen (_cfg_key[key_id]);
  while (++string && isspace (*string))
    {
      /* condition does all the processing */
    }
  value = strdup (string);

  if (!list_append (_cfg->seen[key_id], value))
    {
      delete (value);
    }
  if (NULL != value)
    {
      log_info ("registered '%s %s'", _cfg_key[key_id], value);
    }
  return (NULL != value);
}

/*! Registers information for an FS command blacklist entry
 */
static bool
_cfg_register_fs_blacklist_entry (const char *string)
{
  return _cfg_register_key_value_entry (CFG_KEY_FS_BLACKLIST, string);
}

/*! Registers information for an option entry
 */
static bool
_cfg_register_option_entry (const char *string)
{
  int i = 0;
  bool found = false;
  const char* value = NULL;

  value = string + strlen (_cfg_key[CFG_KEY_OPTION]);
  while (++value && isspace (*value))
    {
      /* condition does all the processing */
    }

  for (i=0; i<num_of (_opt_vals); ++i)
    {
      if (0 == strcmp_c (value, _opt_vals[i]))
        {
          found = true;
          break;
        }
    }
  if (!found)
    {
      log_info ("unknown option: '%s'", value);
      return false;
    }
  return _cfg_register_key_value_entry (CFG_KEY_OPTION, string);
}

/*!
 */
static void
_cfg_probe_no_op (list *dev_list)
{
  return;
}

/* Reuse the SCSI attach function because it does all we need to do,
   at least for as long as it caters to the sanei_scsi API.
 */
#define _cfg_net_attach _cfg_scsi_attach

/*!
 */
static void
_cfg_probe_net (list *dev_list)
{
  char* buf = NULL;
  char* rbuf = NULL;

  size_t size = 0;

  ssize_t n = 0;
  uint16_t id = 0;
  uint8_t status;

  void* net = NULL;
  int sock = -1;

  int i = 0;
  char* cp;

  list *registry = _cfg->seen[CFG_KEY_NET];
  cfg_net_info *info = NULL;
  list_entry *cur;

  require (dev_list);
  if (!registry) return;
  
  net = net_init (NULL, NULL);
  if (net) sock = net_get_sock (net);

  if (!net || 0 > sock)
    {
      cfg_set (_cfg, CFG_KEY_NET, false);
      return;
    }
  
  /* construct string from list of scanners */
  
  cur = registry->cur;
  list_reset (registry);
  while ((info = list_next (registry)))
    {
      /* +1 for separator */
      size += strlen (info->spec) + 1;
    }
  registry->cur = cur;

  buf = t_malloc (size+1, char); /* +1 for NULL termination character */
  if (!buf)
    {
      cfg_set (_cfg, CFG_KEY_NET, false);
      return;
    }
  memset (buf, 0, size+1);
  
  cur = registry->cur;
  list_reset (registry);
  while ((info = list_next (registry)))
    {
      strcat (buf, info->spec);
      strcat (buf, "\n");
    }
  registry->cur = cur;

  log_info ("Probe network:\n%s", buf);

  /* send string of NULL separated scanner ips to the network plugin */
  for (i=0; i<size; ++i) if ('\n' == buf[i]) buf[i] = '\0';
  n = ipc_send (sock, 0, TYPE_LIST, size, buf);
  delete (buf);

  if (n != size)
    {
      log_info ("Communication error occurred. Disabling network plugin.");
      cfg_set (_cfg, CFG_KEY_NET, false);
      return;
    }

  /* receive a string of available network scanners */
  n = -1;
  int tries = 3;
  while (0 > n && 0 < tries)
    {
      n = ipc_recv (sock, &id, &status, (void **) &rbuf);
      --tries;
    }

  /* bail if no network scanners were found or something bad happened */
  if (0 >= n || 0 == strlen (rbuf) || status != STATUS_OK)
    {
      log_info ("No network scanners detected. Disabling network plugin.");
      cfg_set (_cfg, CFG_KEY_NET, false);
      delete (rbuf);
      return;
    }

  /* process the scanner listing string */
  _cfg_dev_list = dev_list;
  _cfg_dev_key = _cfg_key[CFG_KEY_NET];

  cp = rbuf;
  for (i=0; i<n; ++i)
    {
      if ('\0' == rbuf[i])
        {
          log_info ("Detected network scanner: %s", cp);
          _cfg_net_attach (cp);
          cp = rbuf + i + 1;
        }
    }

  _cfg_dev_key = NULL;
  _cfg_dev_list = NULL;

  delete (rbuf);

  return;
}

/*!
 */
static void
_cfg_probe_scsi (list *dev_list)
{
  list *registry = _cfg->seen[CFG_KEY_SCSI];
  cfg_scsi_info *info = NULL;
  list_entry *cur;

  require (dev_list);
  if (!registry) return;

  cur = registry->cur;
  list_reset (registry);
  while ((info = list_next (registry)))
    {
      _cfg_dev_list = dev_list;
      _cfg_dev_key = _cfg_key[CFG_KEY_SCSI];
      sanei_scsi_find_devices (info->vendor, info->model, NULL,
                               -1, -1, -1, -1, _cfg_scsi_attach);
      _cfg_dev_key = NULL;
      _cfg_dev_list = NULL;
    }
  registry->cur = cur;

  return;
}

static void
_cfg_probe_usb (list *dev_list)
{
  list *registry = _cfg->seen[CFG_KEY_USB];
  cfg_usb_info *info = NULL;
  list_entry *cur;

  require (dev_list);
  if (!registry) return;

  cur = registry->cur;
  list_reset (registry);
  while ((info = list_next (registry)))
    {
      _cfg_dev_list = dev_list;
      _cfg_dev_key = _cfg_key[CFG_KEY_USB];
      sanei_usb_find_devices (info->vendor, info->product, _cfg_usb_attach);
      _cfg_dev_key = NULL;
      _cfg_dev_list = NULL;
    }
  registry->cur = cur;

  return;
}

/*!
 */
static void
_cfg_probe_interpreter (list *dev_list)
{
  list *registry = _cfg->seen[CFG_KEY_INTERPRETER];
  cfg_interpreter_info *info = NULL;
  list_entry *cur;

  require (dev_list);
  if (!registry) return;

  cur = registry->cur;
  list_reset (registry);
  while ((info = list_next (registry)))
    {
      _cfg_dev_list = dev_list;
      _cfg_dev_key = _cfg_key[CFG_KEY_INTERPRETER];
      sanei_usb_find_devices (info->vendor, info->product, _cfg_usb_attach);
      _cfg_dev_key = NULL;
      _cfg_dev_list = NULL;
    }
  registry->cur = cur;

  return;
}

static SANE_Status
_cfg_scsi_attach (SANE_String_Const dev_name)
{
  SANE_String name = NULL;
  size_t  len_name = (strlen (_cfg_dev_key) + strlen (DEV_NAME_SEP_STR)
                      + strlen (dev_name) + 1);

  name = t_malloc (len_name, SANE_Char);
  if (name)
    {
      SANE_Status s = SANE_STATUS_GOOD;

      strcpy (name, _cfg_dev_key);
      strcat (name, DEV_NAME_SEP_STR);
      strcat (name, dev_name);

      s = _cfg_attach (name, _cfg_dev_list);
      if (SANE_STATUS_NO_MEM == s)
        {
          delete (name);
        }
      return s;
    }
  return SANE_STATUS_NO_MEM;
}

static SANE_Status
_cfg_usb_attach (SANE_String_Const dev_name)
{
  const char *sanei_usb_prefix = "libusb:";

  SANE_String name = NULL;
  size_t  len_name = (strlen (_cfg_dev_key) + strlen (DEV_NAME_SEP_STR)
                      + strlen (dev_name) + 1);

  if (0 == strncmp_c (dev_name, sanei_usb_prefix, strlen (sanei_usb_prefix)))
    {
      len_name -= strlen (sanei_usb_prefix);
      dev_name += strlen (sanei_usb_prefix);
    }

  name = t_malloc (len_name, SANE_Char);
  if (name)
    {
      SANE_Status s = SANE_STATUS_GOOD;

      strcpy (name, _cfg_dev_key);
      strcat (name, DEV_NAME_SEP_STR);
      strcat (name, dev_name);

      s = _cfg_attach (name, _cfg_dev_list);
      if (SANE_STATUS_NO_MEM == s)
        {
          delete (name);
        }
      return s;
    }
  return SANE_STATUS_NO_MEM;
}

/*! Creates a SANE_Device and attaches it to the \a dev_list.
 *
 *  While perhaps not particularly user-friendly, it is perfectly okay
 *  to attach a SANE_Device with the vendor, model and type fields set
 *  to something that does not exactly correspond with the device that
 *  is dangling on the other end of the connection.
 *
 *  We may not be able to get reliable information for several reasons,
 *  not in the least because of (intermittent?) failure to communicate
 *  with the device.
 *
 *  Returns SANE_STATUS_NO_MEM if a SANE_Device could not be created
 *  and added to the \a dev_list, SANE_STATUS_GOOD otherwise.  Note
 *  that in the latter case only the name field is guaranteed to be
 *  not \c NULL.
 */
static SANE_Status
_cfg_attach (SANE_String_Const dev_name, list *dev_list)
{
  SANE_Device *dev = t_malloc (1, SANE_Device);

  require (dev_name);

  if (!dev || !list_append (dev_list, dev))
    {
      delete (dev);
      return SANE_STATUS_NO_MEM;
    }

  dev->name   = dev_name;

  dev->vendor = _cfg_get_vendor (dev_name);
  dev->model  = _cfg_get_model (dev_name);
  dev->type   = _cfg_get_type (dev_name);

  return SANE_STATUS_GOOD;
}

/*! Returns a best effort vendor name for a device.
 *
 *  \todo Remove hard-coding (as soon as we support non-Epson
 *        devices).
 */
static SANE_String_Const
_cfg_get_vendor (SANE_String_Const dev_name)
{
  return strdup ("Epson");
}

/*! Returns a best effort model name for a device.
 */
static SANE_String_Const
_cfg_get_model (SANE_String_Const dev_name)
{
  SANE_String_Const model = NULL;
  SANE_Status status = SANE_STATUS_GOOD;

  char *fw_name = NULL;
  channel *ch = NULL;

  require (dev_name);

  ch = channel_create (dev_name, &status);
  if (!ch || SANE_STATUS_GOOD != status)
    {
      err_minor ("%s", sane_strstatus (status));
    }
  else
    {
      ch->open (ch, &status);
      if (SANE_STATUS_GOOD == status)
        {
          fw_name = get_fw_name (ch);
        }
      ch->close (ch, NULL);
      ch = ch->dtor (ch);
    }

  log_info ("F/W name: '%s'", fw_name);

  model = model_info_cache_get_model (fw_name);
  delete (fw_name);

  return model;
}

/*! Returns a best effort type description for a device.
 *
 *  \todo Figure out how to deal with the predefined strings in the
 *        face of devices that have a flatbed and ADF and/or TPU.
 *        Then there is also the various ideas people have about
 *        multi-function peripherals.  Are these SPC's, all-in-ones,
 *        MFPPs or even plain scanners with both a flatbed and ADF?
 */
static SANE_String_Const
_cfg_get_type (SANE_String_Const dev_name)
{
  return strdup ("flatbed scanner");
}

/*! Divines whether a certain interpreter is usable.
 */
static bool
_cfg_have_interpreter (const char *library, const char *firmware)
{
  require (library);

  return true;
}


static void
_cfg_net_dtor (void *self)
{
  cfg_net_info *p = self;

  if (!p) return;

  const_delete (p->spec, char *);
  delete (p);
}

static void
_cfg_scsi_dtor (void *self)
{
  cfg_scsi_info *p = self;

  if (!p) return;

  const_delete (p->vendor, char *);
  const_delete (p->model , char *);
  delete (p);
}

static void
_cfg_interpreter_dtor (void *self)
{
  cfg_interpreter_info *p = self;

  if (!p) return;

  const_delete (p->library , char *);
  const_delete (p->firmware, char *);
  delete (p);
}
