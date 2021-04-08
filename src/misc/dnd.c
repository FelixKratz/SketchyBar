static bool getDoNotDisturb() {
  bool doNotDisturb;
  NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
  bool isBigSur = version.majorVersion == 11 || (version.majorVersion == 10 && version.minorVersion > 15);
  if (isBigSur) {
    // On big sur we have to read a plist from a plist...
    NSData* dndData = [[[NSUserDefaults alloc] initWithSuiteName:@"com.apple.ncprefs"] dataForKey:@"dnd_prefs"];
    // If there is no DND data let's assume that we aren't in DND
    if (!dndData) return false;

    NSDictionary* dndDict = [NSPropertyListSerialization
            propertyListWithData:dndData
                         options:NSPropertyListImmutable
                          format:nil
                           error:nil];
    // If the dnd data isn't a valid plist, again assume we aren't in DND
    if (!dndDict) return false;

    NSDictionary* userPrefs = [dndDict valueForKey:@"userPref"];
    if (userPrefs) {
      NSNumber* dndEnabled = [userPrefs valueForKey:@"enabled"];
      // If the user pref has it set to enabled
      if ([dndEnabled intValue] == 1) return true;
    }

    NSDictionary* scheduledPrefs = [dndDict valueForKey:@"scheduledTime"];
    if (scheduledPrefs) {
      NSNumber* scheduleEnabled = [scheduledPrefs valueForKey:@"enabled"];
      NSNumber* start = [scheduledPrefs valueForKey:@"start"];
      NSNumber* end = [scheduledPrefs valueForKey:@"end"];
      // If the schedule is enabled, we need to manually determine if we fall in the start / end interval
      if ([scheduleEnabled intValue] == 1 && start && end) {
        NSDate* now = [NSDate date];
        NSCalendar *calendar = [NSCalendar currentCalendar];
        NSDateComponents *components = [calendar components:(NSCalendarUnitHour | NSCalendarUnitMinute) fromDate:now];
        NSInteger hour = [components hour];
        NSInteger minute = [components minute];
        NSInteger current = (hour * 60) + minute;

        NSInteger startInt = [start intValue];
        NSInteger endInt = [end intValue];
        // Normal way round, start is before the end
        if (startInt < endInt) {
          // Start is inclusive, end is exclusive
          if (current >= startInt && current < endInt) return true;
        } else if (endInt < startInt) {
          // The end can also be _after_ the start making the DND interval loop over midnight
          if (current >= startInt) return true;
          if (current < endInt) return true;
        }
      }
    }

    // Not manually enabled, not enabled due to schedule
    return false;
  } else {
    // Older than big sur we can just read the pref directly
    doNotDisturb = [[[[NSUserDefaults alloc] initWithSuiteName:@"com.apple.notificationcenterui"] objectForKey:@"doNotDisturb"] boolValue];
  }
  return doNotDisturb;
}
