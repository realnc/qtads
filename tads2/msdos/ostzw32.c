/* 
 *   Copyright (c) 1987, 2002 by Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  ostzw32.c - osifc timezone routines for Win32
Function
  
Notes
  
Modified
  08/05/12 MJRoberts  - Creation
*/

#include <windows.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "std.h"
#include "os.h"

extern void safe_strcpy(char *dst, size_t dstlen, const char *src);

/* list of mappings from Windows zone names to Olson zone names */
typedef struct win_zone_map_t win_zone_map_t;
struct win_zone_map_t
{
    const WCHAR *win_name;   /* Windows zone name ("Pacific Standard Time") */
    const char *tz_name;   /* zoneinfo (Olson) name ("America/Los_Angeles") */
    const char *st_abbr;              /* standard time abbreviation ("PST") */
    const char *dst_abbr;             /* daylight time abbreviation ("PDT") */
};
static const win_zone_map_t S_win_zone_map[] = {
/* 
 *   include the mapping file we generate from the Unicode CLDR file
 *   windowsZones.xml - this contains a list of { "win name", "olson name" }
 *   constant data definitions that will populate this array for us
 */
#include "windowsZones.h"
};

/* find a Windows zone name in the zone map list */
static const win_zone_map_t *win_to_zoneinfo(const wchar_t *win_name)
{
    /* search the list for the given name */
    const win_zone_map_t *z;
    for (z = S_win_zone_map ; z->win_name != 0 ; ++z)
    {
        if (wcscmp(z->win_name, win_name) == 0)
            return z;
    }

    /* didn't find it */
    return 0;
}

/* 
 *   Generate a timezone abbreviation from a windows zone name.  This is an
 *   approximation at best; we use this as a last resort when we can't find
 *   the zone in our zone list.  We'll simply pull out the initials of the
 *   words in the zone name, unless it already looks like an abbreviation, in
 *   which case we'll just return it as-is.
 */
static void gen_tz_abbr(char *abbr, size_t abbrlen, const wchar_t *name)
{
    /* if the name doesn't have any spaces, assume it's already abbreviated */
    if (wcschr(name, ' ') == 0)
    {
        /* no spaces, so assume it's already abbreviation and just copy it */
        for ( ; abbrlen > 1 && *name != 0 ; --abbrlen)
            *abbr++ = (char)*name++;
    }
    else
    {
        /* pull out the first letter of each word */
        int sp = TRUE;
        for ( ; abbrlen > 1 && *name != 0 ; ++name)
        {
            if (*name == ' ')
                sp = TRUE;
            else if (sp)
            {
                *abbr++ = (char)*name;
                sp = FALSE;
            }
        }
    }
    if (abbrlen > 0)
        *abbr = '\0';
}

/*
 *   Get the zoneinfo (Olson) timezone name for the local machine/process.
 */
int os_get_zoneinfo_key(char *name, size_t namelen)
{
    TIME_ZONE_INFORMATION tz;
    const win_zone_map_t *z;

    /* get the Windows time zone information */
    if (GetTimeZoneInformation(&tz) != TIME_ZONE_ID_INVALID
        && (z = win_to_zoneinfo(tz.StandardName)) != 0)
    {
        /* success - return the zoneinfo name mapping */
        safe_strcpy(name, namelen, z->tz_name);
        return TRUE;
    }

    /* there's no time zone information from Windows; return failure */
    return FALSE;
}

/*
 *   Get the current settings for the local time zone 
 */
int os_get_timezone_info(struct os_tzinfo_t *info)
{
    TIME_ZONE_INFORMATION tz;
    int mode;

    /* initialize all fields to zero */
    memset(info, 0, sizeof(*info));

    /* get the Windows time zone information */
    if ((mode = GetTimeZoneInformation(&tz)) != TIME_ZONE_ID_INVALID)
    {
        const win_zone_map_t *z;

        /* check to see if standard/daylight change rules are present */
        if (mode != TIME_ZONE_ID_UNKNOWN && tz.StandardDate.wMonth != 0)
        {
            /* 
             *   We have both daylight and standard time.  Set the GMT offset
             *   for each; Windows gives us to this as minutes west of GMT,
             *   and we want seconds east, so negate it and multiply by 60
             *   seconds per minutes.
             */
            info->std_ofs = -(tz.Bias + tz.StandardBias) * 60;
            info->dst_ofs = -(tz.Bias + tz.DaylightBias) * 60;

            /* check whether we're on standard or daylight right now */
            info->is_dst = (mode == TIME_ZONE_ID_DAYLIGHT);
        }
        else
        {
            /* there's no daylight rule, so we're always on standard time */
            info->std_ofs = info->dst_ofs = -tz.Bias * 60;
            info->is_dst = FALSE;
        }

        /* 
         *   If we have recurring standard/daylight rules, set the rules.
         *   Recurring rules are indicated by wYear == 0; a non-zero wYear
         *   means that the rule occurs on that exact date only.  We have no
         *   way to represent once-only rules, so we ignore them and indicate
         *   that no rules are available.
         */
        if (mode != TIME_ZONE_ID_UNKNOWN
            && tz.StandardDate.wMonth != 0
            && tz.StandardDate.wYear == 0
            && tz.DaylightDate.wYear == 0)
        {
            /* set 'dst_start' to the DaylightDate rule */
            info->dst_start.month = tz.DaylightDate.wMonth;
            info->dst_start.week = tz.DaylightDate.wDay;
            info->dst_start.day = tz.DaylightDate.wDayOfWeek + 1;
            info->dst_start.time =
                tz.DaylightDate.wHour*60*60 + tz.DaylightDate.wMinute*60;

            /* set 'dst_end' to the StandardTime rule */
            info->dst_end.month = tz.StandardDate.wMonth;
            info->dst_end.week = tz.StandardDate.wDay;
            info->dst_end.day = tz.StandardDate.wDayOfWeek + 1;
            info->dst_end.time =
                tz.StandardDate.wHour*60*60 + tz.StandardDate.wMinute*60;
        }

        /* look up the windows zone in our list to get the abbreviations */
        if ((z = win_to_zoneinfo(tz.StandardName)) != 0)
        {
            /* found it - return the mapped abbreviations */
            safe_strcpy(info->std_abbr, sizeof(info->std_abbr), z->st_abbr);
            safe_strcpy(info->dst_abbr, sizeof(info->dst_abbr), z->dst_abbr);
        }
        else
        {
            /* 
             *   we couldn't find a mapping; generate the abbreviation from
             *   the windows zone name by pulling out the initials (for
             *   example, "Pacific Standard Time" -> "PST")
             */
            gen_tz_abbr(info->std_abbr, sizeof(info->std_abbr),
                        tz.StandardName);
            gen_tz_abbr(info->dst_abbr, sizeof(info->dst_abbr),
                        tz.DaylightName);
        }

        /* success */
        return TRUE;
    }
    else
    {
        /* failed */
        return FALSE;
    }
}

