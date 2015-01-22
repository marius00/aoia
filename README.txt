--------------------------------------------------------------------------------

                         ANARCHY ONLINE ITEM ASSISTANT
                             http://ia.frellu.net

--------------------------------------------------------------------------------

Installation:
=============
Unzip and run from your location of choice. To avoid possible issues, extract 
to C:\aoia or similar. Running from the Desktop or from inside Program Files, 
may cause problems (in the sense that AOIA wont work) on some systems.

Upgrading:
==========
When upgrading from a previous version I recommend making a copy of your old 
ItemAssistant.db file just to be on the safe side.
If the automated migration to the new version creates wierd results please
send a detailed description of the problem and attach both the old and the 
newly converted ItemAssistant.db file.

Usage:
======
Online help system is available at http://ia-help.frellu.net

Known Issues:
=============
    * If you have copied your AO installation to a new directory and is now
      playing from the new location, Item Assistant will not be able to detect
      that the old installation is not valid any more.
      To fix this you need to open the file called ItemAssistant.conf and either
      - Change the AOPath setting to the new location of your AO installation,
        or,
      - Delete the AOPath setting. Item Assistant will then ask you to specify
        the location on the next startup.
    * AOIA needs to run with the same, or higher, level of privileges on your 
      computer as the AO client itself is running with. If you run the AO client
      as administrator, you have to do the same with AOIA.
    * Sorting items on the "Backpack" column generates weird results.
    * Moving a backpack or item while AOIA is _not_ running, can result in 
      duplicate items in the AOIA database. Re-open all affected backpacks to 
      rescan the content and fix the sync issue.

Aknowledgements:
================
This application is in no way only due to my own efforts. Some people who 
have contributed either directly or indirectly include:
    * Morten        - Author of original AOIA
    * kennethx1     - Moral support and code.
    * auno          - aoppa source was a big help.
    * morb, gnarf   - clicksaver rocks and showed me the technique that made 
                      it all possible.
    * RobotJunk     - Multi-thread patch for the hook dll and Client-to-Server 
                      message hooking as well as all the item-transfer message 
                      decoding.
    * Gremlin       - For being a guinea pig.
    * Mordwin       - Integration with CharacterParser and bugfixes.
    * Ken Leang     - For fixing the Alappa Gems part of the identify view.
    * Jellyfunk     - For adding support for new AO prefs folder.
    * Everyone else that has provided feedback and done testing.

Change Log:
===========
Version 1.1.5                                        Release Date: ??/03/2013
    * Fixed issue with recursive bank content after server merge (v18.6).
    * Removed obsolete "player shop monitor" view.
    * Removed dimension support from "summary view", "inventory view" and
      "pattern matcher view".
    * Added support for the new preference folder location of AO.

Version 1.1.4                                       Release Date: 15/12/2012
    * Fixed issue with connecting to the AO client after the 18.5 patch.
    * Fixed parsing of the AO DB to handle the updates of the 18.5 patch.

Version 1.1.3                                       Release Date: 09/12/2011
    * Fixed an SQL error while ripping the AO database.
    * Fixed the isse with the application not being able to hook the AO client.

Version 1.1.2                                       Release Date: 01/12/2011
    * Fixed a crash during startup.
    * Fixed an SQL bug when updating toon names.
    * Added logging to the AO hook. Now logs to <temp>/aoia_hook.log.
    * Logging is now enabled by default and logs to <temp>/aoia.log.

Version 1.1.1                                       Release Date: 14/11/2011
    * Fixed issue with dialog popping up on each restart.

Version 1.1.0                                       Release Date: 11/11/2011
    * Added a new column called ContainerID to the CSV export.
    * Unified all CSV exports to have the same format.
    * Numbers shown on the summary page should be a bit more readable.
    * Added crashdump generation to help with troubleshooting issues.
    * Added command on the item context menu in the inventory page for deleting
      the selected item(s).
    * Fixed bug when restoring window position. Now resets to default if the 
      entire window is outside the current desktop.
    * Fixed performance issue with some more disappearing database indexes.

Version 1.0.1                                       Release Date: 16/9/2011
    * Added DB index to fix issue with speed and CPU usage while searching for
      items.
    
Version 1.0.0                                       Release Date: 10/9/2011
    * Fixed bug 2947010. Generating the script with items for ingame display
      now use the correct linebreak tag.
    * Fixed bug 2802231 in the find panel and in the PB filter panel. Toon 
      list is now reloaded as it is opened.
    * Feature 2059217: The window location is now recorded and restored 
      to/from the conf file. 
    * Opening an item using Xyphos.com will now also supply the QL of the item
      so the stats are shown correctly.
    * Improved ripping of items database so it supports newer version of AO.
    * New summary view shows levels and amount of credits for all registered 
      toons.
    * Created a basic installer.

Version 0.9.7                                       Release Date: 26/1/2010
    * Fixed org city output for CharacterParser integration. Does not dump
      contract message on zone since it doesn't contain anything useful anyway.
    * Includes UAC manifest in EXE file. Hopefully this should make some 
      compatibility problems with Vista and 7 go away.

Version 0.9.6                                       Release Date: 16/1/2010
    * Changed link to AO Mainframe.
    * Updated SQLite to version 3.6.22 to resolve some issues.
    * Fixed database parser bugs. The symptom was that the application was 
      unable to track certain items.
    * Application now outputs files that can be opened with the CharacterParser
      application.

Version 0.9.5                                       Release Date: 3/7/2009
    * Fixed database parser so the app will run on Windows XP again.
    * Fixed message handler so the app once again connects to the AO client 
      and can track items.

Version 0.9.4                                       Release Date: 30/6/2009
    * Added support for the new database in v18.1.
    * The application is no longer using the registry to store its settings.
      Settings are stored in the ItemAssistant.conf file.
    * Enhanced item listing so the container column shows if an item is 
      equipped, and at which tab-panel it is.
    * Added baloon warning popup when application is started after the AO 
      client was started since this may lead to the database being out of sync.
    * Improved statusbar for the Inventory view.
    * The application will now remember which dimension you chose last time you
      changed dimension in either the find panel or the pattern matcher.
    * Added "Copy Item Name(s)" to the item context menu.
    * Bug Fix: Fixed weird encoding of ampersands in backpack names.
    * Bug Fix: Exporting all items from a toon should not put multiple quotes
      around the backpack names any more.
    * Bug Fix: Fixed buffer overflow with long backpack names.

Version 0.9.3                                       Release Date: 4/5/2009
    * Added support for NPC trade-in quest rewards in the item tracking.
    * Now parsing the 18.0.1 database correctly.
    * Find panel now filters on dimension as well.
    * Added support for Xyphos.com for item stats/links.
    * The report in the Pattern Matcher now links to the loot table for the 
      boss.
    * It is now possible to export all items (excluding backpacks) from a toon
      by right-clicking the toon name in the tree-view.
    * Improved performance when ripping the AODB.
    * Bug Fix: Identify view will now sort (right hand side list).
    * Bug Fix: Toonlist in the find-panel and pattern-matcher should now be 
      sorted.

Version 0.9.2                                       Release Date: 20/2/2009
    * Enhanced the player-shop monitor to refresh the view when something 
      changes on disc. Cleaned up file system monitoring.
    * Added more logging to make it easier to submit bug reports.
    * Added hooking of messages sent from the client. Up until now only 
      messages from the server have been intercepted.
    * Inventory viewer now tracks changes to the players inventory and 
      backpacks as they are being changed by the user. This should reduce the 
      number of full reparsings you would need to do.
    * Added "Identify" view that lets you quickly find any bio clumps or 
      compounds from Alappa based on what they will turn into once 
      tradeskilled.
    * Experiemental: Added support for recording a toons dimension ID.
    * Fixed a bug with recording playershop IDs.

Version 0.9.1                                       Release Date: 25/10/2008
    * Added experimental tracking of player-shop contents.
    * Updated the help menu a bit.
    * Reduced lag by making the AO hook multi-threaded and cache relevant 
      information for a longer period of time.
    * Fixed pause button and tray popup in the Player Shop Monitor.
    * Fixed problem with some Inamorata Rifles showing an empty name.

Version 0.9.0                                       Release Date: 09/10/2008
    * Now works on both WindowsXP as well as Vista (32 and 64 bit) without the
      need to be launched in compatibility mode.
    * Added "-log" commandline option.
    * Playershop monitor added.
    * The application will now parse the original AO database to create its 
      own item database.
    * GUI cleanup.
    * Some context menu commands now work with multiple selection of items.
    * Copying item references to AO script format now also writes the result
      to a script called 'aoia' to the scripts folder of Anarchy Online.
    * Added CSV export to inventory context menu.
    * Specifying minimum and maximum QL level when looking for an item is now 
      possible.
    * Items are now shown with the correct interpolated name.
    * Fixed an issue that would cause the character ID to be shown instead of 
      the character name.
    * The Pattern Matcher now has an option to exclude already assembled ABCD 
      patterns from the availability calculation.

Version 0.8.4                                       Release Date: 31/03/2008
    * Updated DB scheme to fix issue with 32 bit unsigned character IDs.
    * Added code to update database file from previous version to the current
      version.
    * The ItemAssistant.db is created during startup if it is missing.
    * Added sorting of toons and named backpacks in the inventory tree-view.
    * Updated URL to aomarket.
    * Fixed a crashbug.
    * General code cleanup.
    * Updated project to Visual Studio 2008.

Version 0.8.3                                       Release Date: 20/03/2008
    * Fixed crash bug with zoning and login after 17.8.1.
    * Inventory tree-view now has a node for the "Social" tab.
    * Added context menu to items. You are now able to show itemstats using 
      either auno.org or aomainframe.net. You can also copy the item as a 
      HTML link or VBB link.
    * Updated item database to version 17.8.1.

Version 0.8.2                                       Release Date: 16/01/2007
    * Automatic detection of toon names. Removed possibility to rename a toon 
      name manualy. Names are detected on zone.
    * Pressing the applications minimize or close button now sends the 
      application to the system tray instead. To actually exit the application,
      select "Exit" from the tray menu or the file menu.
    * Updated item database to version 17.0.3.

Version 0.8.1                                       Release Date: 24/11/2006
    * Fixed a bug in the Inventory view that caused the last character of all 
      itemnames to disappear.

Version 0.8.0                                       Release Date: 7/11/2006
    * Updated the item DB to match v16.4.1 of Anarchy Online.
    * Upgraded to use latest version of SQLite (v3.3.8). You should still be 
      able to use your old DB file.
    * Converted to using UNICODE wherever I can. This means support for the 
      UTF-8 encoded XML files that AO is using, is in place. I have only 
      tested with a Norwegian characterset, but I suspect Russian and 
      whatnot should work as well. The downside is that the app will probably 
      NOT work on Windows 95/98... but who cares?
    * The about box now actually contains some information. :P
    * Removed the "Bot Export" view. It didn't do squat anyway.
    * The "Message View" is now only available in the debug version like 
      originally intended.
    * Now supports a new command line argument called "-db". It specifies 
      what database file to use.
      Example: ItemAssistant.exe -db C:\my_rk2_items.db
    * Re-thought the whole pattern-matcher concept with some new ideas and 
      valuable input by "Senlee".
      (You want YOUR ideas heard? Use the feedback form!)
    * Redesigned the Pattern Matcher GUI. Now have a listview containing the 
      name and availability of all the pocket bosses.
    * Selecting a pocket boss in the list will bring up a report outlining 
      where you have pattern pieces for this pockect boss stored. (Both toon 
      names and backpack names and locations.)
    * The application is now multithreaded. A seperate thread is spawned to 
      calculate the availability column.

Version 0.6.2                                       Release Date: 27/04/2006
    * Pattern Matcher and Inventory list-view now shows proper container names
      for location of pattern pieces.

Version 0.6.1                                       Release Date: 16/04/2006
    * Pattern Matcher has had some work done on it. It no longer does a query 
      on startup, so starup speed has improved. Also the pattern and toon 
      combo-boxes work.
    * Zoning now removes items in the "unknown" container of the current 
      charater.

Version 0.6                                         Release Date: 10/12/2005
    * Backpack parsing to match new packet format in AO 16.1.
    * First version of the info panel (CTRL+I). Still lots of work left on it.

Version 0.5                                         Release Date: 20/09/2005
    * Release build.
    * AO Item database updated to version 16.0.2
    * Info panel (CTRL-I) that shows details about selected item in the 
      Inventory view.
    * Zoning now auto-deletes items in the "unknown" tree node.
    * Deleting the items in the "unknown" tree node should no longer delete 
      all items.

Version 0.4 (and earlier)                           Release Date: BC
    * Initial test builds.
    * Debug builds.
