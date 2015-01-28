<?php include("shared_start.php"); ?>

<h1>AO Item Assistant : The Player Shop Monitor</h1>

<h2>Introduction</h2>
<p>The purpose of this plugin is two-fold:
<ul>
<li>Provide a browser for all the playershop log files in your AO installation</li>
<li>Give you a notification when you have made a sale without the need to go to 
the shop and check manually.</li>
</ul>
</p>

<center>
<a href="playershop.jpg" target="_blank"><img src="playershop-50.jpg" /></a>
</center>

<h2>The GUI Explained</h2>
<p>The treeview on the left contains all the accounts found in your AO folder
with sub-nodes for each toon on that account.
<i>(If you only see numbers you need to log on that toon whila AOIA is 
running so it can detect the name of the toon.)</i></p>

<p>Selecting a node in the treeview will update the listview on the right with
all items currently in the playershop log for that account or toon.</p>

<p>If a sale takes place on the toon you are playing, a popup message will
appear in the system tray with information about the transaction the next
time you zone.</p>

<img src="pause_monitoring_button.png" style="float:right; padding-left:5px" />

<p>In the toolbar there is a toggle-button called <b>Pause Monitoring</b> that 
lets you turn off this plugin temporarily. While it is pressed, you will not 
receive any sales notifications nor updates to the view itself.</p>

<?php include("shared_end.php"); ?>