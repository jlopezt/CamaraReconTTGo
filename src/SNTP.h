/***********************************************************
 *    Modulo de gestion de SNTP
 * 
 *    https://github.com/espressif/arduino-esp32/issues/680
 ***********************************************************/
/*****************************************************
The format of TZ values recognized by tzset() is as follows:
stdoffset[dst[offset][,rule]]copy to clipboard

std and dst
Indicate no fewer than three, but not more than TZNAME_MAX, bytes that are the designation for the standard (std) and daylight savings (dst) time zones. If more than TZNAME_MAX 
bytes are specified for std or dst, tzset() truncates to TZNAME_MAX bytes. Only std is required; if dst is missing, daylight savings time does not apply in this locale. Uppercase 
and lowercase letters are explicitly allowed. Any character except a leading colon (:) or digits, the comma (,), the minus (-), the plus (+), and the NULL character are permitted 
to appear in these fields. The meaning of these letters and characters is unspecified.

offset
Indicates the value that must be added to the local time to arrive at Coordinated Universal Time (UTC). offset has the form: hh[:mm[:ss]] The minutes (mm) and seconds (ss) are 
optional. The hour (hh) is required and may be a single digit. offset following std is required. If no offset follows dst, daylight savings time is assumed to be 1 hour ahead of 
standard time. One or more digits may be used; the value is always interpreted as a decimal number. The hour must be between 0 and 24; minutes and seconds, if present, between 0 
and 59. The difference between standard time offset and daylight savings time offset must be greater than or equal to 0, but the difference may not be greater than 24 hours. Use 
of values outside of these ranges causes tzset() to use the LC_TOD category rather than the TZ environment variable for time conversion information. An offset preceded by a minus 
(-) indicates a time zone east of the Prime Meridian. A plus (+) preceding offset is optional and indicates the time zone west of the Prime Meridian.

rule
Indicates when to change to and back from daylight savings time. The rule has the form: date[/time],date[/time]
The first date describes when the change from standard to daylight savings time occurs and the second date describes when the change back happens. Each time field describes when, 
in current local time, the change to the other time is made.

The format of date must be one of the following:
Jn: The Julian day n (1≤n≤365). Leap days are not counted. That is, in all years—including leap years—February 28 is day 59 and March 1 is day 60. It is impossible to explicitly 
refer to the occasional February 29.
n: The zero-based Julian day (0≤n≤365). Leap days are counted, and it is possible to refer to February 29.
Mm.n.d
The dth day (0≤d≤6) of week n of month m of the year (1≤n≤5, and 1≤m≤12, where week 5 means “ the last d day in month m,” which may occur in either the fourth or the fifth 
week). Week 1 is the first week in which the dth day occurs. Day zero is Sunday.
The time has the same format as offset except that no leading sign, minus (-) or plus (+), is allowed. The default, if time is not given, is 02:00:00.

If dst is specified and rule is not specified by TZ or in LC_TOD category, the default for the daylight savings time start date is M4.1.0 and for the daylight savings time end 
date is M10.5.0.
******************************************************/

/***************************** Defines *****************************/
#ifndef _SNTP_
#define _SNTP_

#define MES_CAMBIO_HORARIO_UP   3 //marzo
#define MES_CAMBIO_HORARIO_DOWN 10 //octubre
#define DOMINGO 7
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <TimeLib.h>  // download from: http://www.arduino.cc/playground/Code/Time
#include <Time.h>
/***************************** Includes *****************************/

class relojClass
{
  private:
  const char* NTP_SERVER = "pool.ntp.org";//"ntp.mydomain.com";
  //const char* TZ_INFO    = "EST5EDT4,M3.2.0/02:00:00,M11.1.0/02:00:00";
  const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/02:00:00";

  struct tm timeinfo;
  /***********************
  Member  Type  Meaning Range
  tm_sec  int seconds after the minute  0-61*
  tm_min  int minutes after the hour  0-59
  tm_hour int hours since midnight  0-23
  tm_mday int day of the month  1-31
  tm_mon  int months since January  0-11
  tm_year int years since 1900  
  tm_wday int days since Sunday 0-6
  tm_yday int days since January 1  0-365
  tm_isdst int Daylight Saving Time flag 
  ***********************/
  public:
  relojClass();
  void inicializaReloj(void);
  int8_t cambioHorario(void) ;
  int8_t diaSemana(void);
  void imprimeDatosReloj(void);
  bool relojPM();
  int hora();
  int minuto();
  int segundo();
  int dia();
  int mes();
  int anno();
  String getHora(void);
  String getFecha(void);
};

extern relojClass reloj;

#endif