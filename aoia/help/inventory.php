<?php include("shared_start.php"); ?>

<h1>AO Item Assistant : The Inventory</h1>


<h2>Introduction</h2>
<p>This is the main view of the application. Here you will be able to browse around
the internal database of all the toons you have been playing while running
Item Assistant in the background.</p>

<center>
<a href="inventoryview.jpg" target="_blank"><img src="inventoryview-50.jpg" /></a>
</center>


<h2>The GUI Explained</h2>
<p>The Inventory View is divided into four different panels. The treeview on 
the left is used to browse through all the toons and their registered 
inventory while the listview in the center is used to list items either 
found through the toon browser or the "Find Item" panel.</p>

<p>The other two panels are displayed by pressing the "Find Item" and
"Item Info" buttons respectively.</p>

<p>"Find Item" will display a panel that allows you to search for a 
named item in a given QL range. Selecting a toon in the dropdown will 
limit the search to that toon.<br />
"Item Info" opens a panel on the right hand side of the window that will
display some internal information of any item you may select in the main 
listview. For your convenience there are three buttons in this view that 
lets you open up the auno, aomainframe or xyphos DB page with the full 
specification of the item.</p>


<h2>Exporting Items</h2>
<p>It is possible to export information from the Inventory View directly
to the clipboard, to an ingame scipt, or to a CSV file.</p>

<img src="menu_export-selection.jpg" style="float:right; padding-left:5px" width="45%" />

<p>This is done by first selecting one or more items you wish to export
and then right-clicking the selection to bring up the context menu. The menu 
contains choices for exporting to the clipboard, to an AO script file,
or a CSV file. The sub-menus lets you choose which of the online database
sites you wish to use for link references.</p>

<img src="menu_toon-operations.jpg" style="float:left; padding-right:5px" width="30%" />

<p>If you wish to export all items from a specific toon, go to the tree-view 
on the left hand side and click the right mouse button on the toon you wish to 
export. From the popup-menu select "Export Items..." and select the desired 
online items database to create links for when prompted.</p>

<div style="clear:left; padding-top:0.5em;" />

<h2>Recording Stats</h2>
<p>By default the application record character stats as files in the "binfiles"
folder. These files can be processed by CharacterParser and the results be used by 
AOSkills2 and implantHelper.</p>

<img src="record_stats_button.png" style="float:left; padding-right:5px" />

<p>To disable recording character stats un-toggle the "Record Stats" button in
the main toolbar.</p>

<p>
<a href="http://www.lastmanut.pwp.blueyonder.co.uk/CharacterParserWebPage/aos2_aoia_cp_ih_edition.html">CharacterParser</a>, 
<a href="http://www.lastmanut.pwp.blueyonder.co.uk/AOSkills2WebPage/aoskills2.html">AOSkills2</a>
and <a href="http://www.lastmanut.pwp.blueyonder.co.uk/implantHelperWebPage/implanthelper.html">ImplantHelper</a>
are tools created by Donald 'Darkbane' Milne.</p>

<?php include("shared_end.php"); ?>