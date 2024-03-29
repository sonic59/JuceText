/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4514)
#endif

#ifndef JUCE_WINDOWS
 #include <sys/time.h>
#else
 #include <ctime>
#endif

#include <sys/timeb.h>

#if JUCE_MSVC
 #pragma warning (pop)

 #ifdef _INC_TIME_INL
  #define USE_NEW_SECURE_TIME_FNS
 #endif
#endif

BEGIN_JUCE_NAMESPACE

//==============================================================================
namespace TimeHelpers
{
    struct tm  millisToLocal (const int64 millis) noexcept
    {
        struct tm result;
        const int64 seconds = millis / 1000;

        if (seconds < literal64bit (86400) || seconds >= literal64bit (2145916800))
        {
            // use extended maths for dates beyond 1970 to 2037..
            const int timeZoneAdjustment = 31536000 - (int) (Time (1971, 0, 1, 0, 0).toMilliseconds() / 1000);
            const int64 jdm = seconds + timeZoneAdjustment + literal64bit (210866803200);

            const int days = (int) (jdm / literal64bit (86400));
            const int a = 32044 + days;
            const int b = (4 * a + 3) / 146097;
            const int c = a - (b * 146097) / 4;
            const int d = (4 * c + 3) / 1461;
            const int e = c - (d * 1461) / 4;
            const int m = (5 * e + 2) / 153;

            result.tm_mday  = e - (153 * m + 2) / 5 + 1;
            result.tm_mon   = m + 2 - 12 * (m / 10);
            result.tm_year  = b * 100 + d - 6700 + (m / 10);
            result.tm_wday  = (days + 1) % 7;
            result.tm_yday  = -1;

            int t = (int) (jdm % literal64bit (86400));
            result.tm_hour  = t / 3600;
            t %= 3600;
            result.tm_min   = t / 60;
            result.tm_sec   = t % 60;
            result.tm_isdst = -1;
        }
        else
        {
            time_t now = static_cast <time_t> (seconds);

          #if JUCE_WINDOWS
           #ifdef USE_NEW_SECURE_TIME_FNS
            if (now >= 0 && now <= 0x793406fff)
                localtime_s (&result, &now);
            else
                zerostruct (result);
           #else
            result = *localtime (&now);
           #endif
          #else

            localtime_r (&now, &result); // more thread-safe
          #endif
        }

        return result;
    }

    int extendedModulo (const int64 value, const int modulo) noexcept
    {
        return (int) (value >= 0 ? (value % modulo)
                                 : (value - ((value / modulo) + 1) * modulo));
    }

    int doFTime (CharPointer_UTF32 dest, const size_t maxChars, const String& format, const struct tm* const tm) noexcept
    {
       #if JUCE_ANDROID
        HeapBlock <char> tempDest;
        tempDest.calloc (maxChars + 2);
        const int result = (int) strftime (tempDest, maxChars, format.toUTF8(), tm);
        if (result > 0)
            dest.writeAll (CharPointer_UTF8 (tempDest.getData()));
        return result;
       #elif JUCE_WINDOWS
        HeapBlock <wchar_t> tempDest;
        tempDest.calloc (maxChars + 2);
        const int result = (int) wcsftime (tempDest, maxChars, format.toWideCharPointer(), tm);
        if (result > 0)
            dest.writeAll (CharPointer_UTF16 (tempDest.getData()));
        return result;
       #else
        return (int) wcsftime (dest.getAddress(), maxChars, format.toUTF32(), tm);
       #endif
    }

    static uint32 lastMSCounterValue = 0;
}

//==============================================================================
Time::Time() noexcept
    : millisSinceEpoch (0)
{
}

Time::Time (const Time& other) noexcept
    : millisSinceEpoch (other.millisSinceEpoch)
{
}

Time::Time (const int64 ms) noexcept
    : millisSinceEpoch (ms)
{
}

Time::Time (const int year,
            const int month,
            const int day,
            const int hours,
            const int minutes,
            const int seconds,
            const int milliseconds,
            const bool useLocalTime) noexcept
{
    jassert (year > 100); // year must be a 4-digit version

    if (year < 1971 || year >= 2038 || ! useLocalTime)
    {
        // use extended maths for dates beyond 1970 to 2037..
        const int timeZoneAdjustment = useLocalTime ? (31536000 - (int) (Time (1971, 0, 1, 0, 0).toMilliseconds() / 1000))
                                                    : 0;
        const int a = (13 - month) / 12;
        const int y = year + 4800 - a;
        const int jd = day + (153 * (month + 12 * a - 2) + 2) / 5
                           + (y * 365) + (y /  4) - (y / 100) + (y / 400)
                           - 32045;

        const int64 s = ((int64) jd) * literal64bit (86400) - literal64bit (210866803200);

        millisSinceEpoch = 1000 * (s + (hours * 3600 + minutes * 60 + seconds - timeZoneAdjustment))
                             + milliseconds;
    }
    else
    {
        struct tm t;
        t.tm_year   = year - 1900;
        t.tm_mon    = month;
        t.tm_mday   = day;
        t.tm_hour   = hours;
        t.tm_min    = minutes;
        t.tm_sec    = seconds;
        t.tm_isdst  = -1;

        millisSinceEpoch = 1000 * (int64) mktime (&t);

        if (millisSinceEpoch < 0)
            millisSinceEpoch = 0;
        else
            millisSinceEpoch += milliseconds;
    }
}

Time::~Time() noexcept
{
}

Time& Time::operator= (const Time& other) noexcept
{
    millisSinceEpoch = other.millisSinceEpoch;
    return *this;
}

//==============================================================================
int64 Time::currentTimeMillis() noexcept
{
    static uint32 lastCounterResult = 0xffffffff;
    static int64 correction = 0;

    const uint32 now = getMillisecondCounter();

    // check the counter hasn't wrapped (also triggered the first time this function is called)
    if (now < lastCounterResult)
    {
        // double-check it's actually wrapped, in case multi-cpu machines have timers that drift a bit.
        if (lastCounterResult == 0xffffffff || now < lastCounterResult - 10)
        {
            // get the time once using normal library calls, and store the difference needed to
            // turn the millisecond counter into a real time.
          #if JUCE_WINDOWS
            struct _timeb t;
           #ifdef USE_NEW_SECURE_TIME_FNS
            _ftime_s (&t);
           #else
            _ftime (&t);
           #endif
            correction = (((int64) t.time) * 1000 + t.millitm) - now;
          #else
            struct timeval tv;
            struct timezone tz;
            gettimeofday (&tv, &tz);
            correction = (((int64) tv.tv_sec) * 1000 + tv.tv_usec / 1000) - now;
          #endif
        }
    }

    lastCounterResult = now;

    return correction + now;
}

//==============================================================================
uint32 juce_millisecondsSinceStartup() noexcept;

uint32 Time::getMillisecondCounter() noexcept
{
    const uint32 now = juce_millisecondsSinceStartup();

    if (now < TimeHelpers::lastMSCounterValue)
    {
        // in multi-threaded apps this might be called concurrently, so
        // make sure that our last counter value only increases and doesn't
        // go backwards..
        if (now < TimeHelpers::lastMSCounterValue - 1000)
            TimeHelpers::lastMSCounterValue = now;
    }
    else
    {
        TimeHelpers::lastMSCounterValue = now;
    }

    return now;
}

uint32 Time::getApproximateMillisecondCounter() noexcept
{
    if (TimeHelpers::lastMSCounterValue == 0)
        getMillisecondCounter();

    return TimeHelpers::lastMSCounterValue;
}

void Time::waitForMillisecondCounter (const uint32 targetTime) noexcept
{
    for (;;)
    {
        const uint32 now = getMillisecondCounter();

        if (now >= targetTime)
            break;

        const int toWait = (int) (targetTime - now);

        if (toWait > 2)
        {
            Thread::sleep (jmin (20, toWait >> 1));
        }
        else
        {
            // xxx should consider using mutex_pause on the mac as it apparently
            // makes it seem less like a spinlock and avoids lowering the thread pri.
            for (int i = 10; --i >= 0;)
                Thread::yield();
        }
    }
}

//==============================================================================
double Time::highResolutionTicksToSeconds (const int64 ticks) noexcept
{
    return ticks / (double) getHighResolutionTicksPerSecond();
}

int64 Time::secondsToHighResolutionTicks (const double seconds) noexcept
{
    return (int64) (seconds * (double) getHighResolutionTicksPerSecond());
}


//==============================================================================
Time JUCE_CALLTYPE Time::getCurrentTime() noexcept
{
    return Time (currentTimeMillis());
}

//==============================================================================
String Time::toString (const bool includeDate,
                       const bool includeTime,
                       const bool includeSeconds,
                       const bool use24HourClock) const noexcept
{
    String result;

    if (includeDate)
    {
        result << getDayOfMonth() << ' '
               << getMonthName (true) << ' '
               << getYear();

        if (includeTime)
            result << ' ';
    }

    if (includeTime)
    {
        const int mins = getMinutes();

        result << (use24HourClock ? getHours() : getHoursInAmPmFormat())
               << (mins < 10 ? ":0" : ":") << mins;

        if (includeSeconds)
        {
            const int secs = getSeconds();
            result << (secs < 10 ? ":0" : ":") << secs;
        }

        if (! use24HourClock)
            result << (isAfternoon() ? "pm" : "am");
    }

    return result.trimEnd();
}

String Time::formatted (const String& format) const
{
    size_t bufferSize = 128;
    HeapBlock<juce_wchar> buffer (128);

    struct tm t (TimeHelpers::millisToLocal (millisSinceEpoch));

    while (TimeHelpers::doFTime (CharPointer_UTF32 (buffer.getData()), bufferSize, format, &t) <= 0)
    {
        bufferSize += 128;
        buffer.malloc (bufferSize);
    }

    return CharPointer_UTF32 (buffer.getData());
}

//==============================================================================
int Time::getYear() const noexcept          { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_year + 1900; }
int Time::getMonth() const noexcept         { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_mon; }
int Time::getDayOfMonth() const noexcept    { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_mday; }
int Time::getDayOfWeek() const noexcept     { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_wday; }
int Time::getHours() const noexcept         { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_hour; }
int Time::getMinutes() const noexcept       { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_min; }
int Time::getSeconds() const noexcept       { return TimeHelpers::extendedModulo (millisSinceEpoch / 1000, 60); }
int Time::getMilliseconds() const noexcept  { return TimeHelpers::extendedModulo (millisSinceEpoch, 1000); }

int Time::getHoursInAmPmFormat() const noexcept
{
    const int hours = getHours();

    if (hours == 0)
        return 12;
    else if (hours <= 12)
        return hours;
    else
        return hours - 12;
}

bool Time::isAfternoon() const noexcept
{
    return getHours() >= 12;
}

bool Time::isDaylightSavingTime() const noexcept
{
    return TimeHelpers::millisToLocal (millisSinceEpoch).tm_isdst != 0;
}

String Time::getTimeZone() const noexcept
{
    String zone[2];

  #if JUCE_WINDOWS
    _tzset();

   #ifdef USE_NEW_SECURE_TIME_FNS
    for (int i = 0; i < 2; ++i)
    {
        char name[128] = { 0 };
        size_t length;
        _get_tzname (&length, name, 127, i);
        zone[i] = name;
    }
   #else
    const char** const zonePtr = (const char**) _tzname;
    zone[0] = zonePtr[0];
    zone[1] = zonePtr[1];
   #endif
  #else
    tzset();
    const char** const zonePtr = (const char**) tzname;
    zone[0] = zonePtr[0];
    zone[1] = zonePtr[1];
  #endif

    if (isDaylightSavingTime())
    {
        zone[0] = zone[1];

        if (zone[0].length() > 3
             && zone[0].containsIgnoreCase ("daylight")
             && zone[0].contains ("GMT"))
            zone[0] = "BST";
    }

    return zone[0].substring (0, 3);
}

String Time::getMonthName (const bool threeLetterVersion) const
{
    return getMonthName (getMonth(), threeLetterVersion);
}

String Time::getWeekdayName (const bool threeLetterVersion) const
{
    return getWeekdayName (getDayOfWeek(), threeLetterVersion);
}

String Time::getMonthName (int monthNumber, const bool threeLetterVersion)
{
    const char* const shortMonthNames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    const char* const longMonthNames[]  = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

    monthNumber %= 12;

    return TRANS (threeLetterVersion ? shortMonthNames [monthNumber]
                                     : longMonthNames [monthNumber]);
}

String Time::getWeekdayName (int day, const bool threeLetterVersion)
{
    const char* const shortDayNames[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    const char* const longDayNames[]  = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

    day %= 7;

    return TRANS (threeLetterVersion ? shortDayNames [day]
                                     : longDayNames [day]);
}

//==============================================================================
Time& Time::operator+= (const RelativeTime& delta)        { millisSinceEpoch += delta.inMilliseconds(); return *this; }
Time& Time::operator-= (const RelativeTime& delta)        { millisSinceEpoch -= delta.inMilliseconds(); return *this; }

Time operator+ (const Time& time, const RelativeTime& delta)        { Time t (time); return t += delta; }
Time operator- (const Time& time, const RelativeTime& delta)        { Time t (time); return t -= delta; }
Time operator+ (const RelativeTime& delta, const Time& time)        { Time t (time); return t += delta; }
const RelativeTime operator- (const Time& time1, const Time& time2) { return RelativeTime::milliseconds (time1.toMilliseconds() - time2.toMilliseconds()); }

bool operator== (const Time& time1, const Time& time2)      { return time1.toMilliseconds() == time2.toMilliseconds(); }
bool operator!= (const Time& time1, const Time& time2)      { return time1.toMilliseconds() != time2.toMilliseconds(); }
bool operator<  (const Time& time1, const Time& time2)      { return time1.toMilliseconds() <  time2.toMilliseconds(); }
bool operator>  (const Time& time1, const Time& time2)      { return time1.toMilliseconds() >  time2.toMilliseconds(); }
bool operator<= (const Time& time1, const Time& time2)      { return time1.toMilliseconds() <= time2.toMilliseconds(); }
bool operator>= (const Time& time1, const Time& time2)      { return time1.toMilliseconds() >= time2.toMilliseconds(); }


END_JUCE_NAMESPACE
