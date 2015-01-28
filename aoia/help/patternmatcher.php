<?php include("shared_start.php"); ?>

<h1>AO Item Assistant : The Pattern Matcher</h1>

<h2>Introduction</h2>
Keeping track of all your pattern-pieces has never been easier. This plugin
search the inventory of all your recorded characters for pattern pieces, and 
lets you quickly see what patterns you can put together, and what toons holds 
all the pieces needed to do so.


<h2>Availability Explained</h2>
<p>The availability of a pocket boss is an indication of how many pieces you 
have available for completing a specific pocket boss. <br />
If the availability is less than 1.00 you do not have enough pieces to complete
it. An availability of 1 or more, indicates a comeplete or completable pocket
boss.</p>

<p>Each of the 4 neccesary pieces contribute 0.25 to the total.
If you have one Abhan and one Dom, this will give you an availability of 0.50.
The excact same availability will be the result if you have one Bothar and one 
Chi piece. However, having two Abhan pieces will only give an availability of 
0.25.</p>


<h2>The GUI Explained</h2>
<p>On the lefthand side of the pattern matcher GUI you will see this dialog.
Below is a short description of the individual GUI elements.</p>

<img src="patterns.jpg" width="202" height="542" style="float:left; margin:5px;" />

<p><b>A</b> : This determine which dimension you want to search for pocket 
boss patterns on.</p>

<p><b>B</b> : This lets you restrict your search for pattern pieces to a single
toon, or if the "-" is chosen <b>all toons</b> on the currently selected 
dimension. All toons ("-") is the default.</p>

<p><b>C</b> : Selecting something other than the "Show All" option, will limit
the number of pocket bosses to be shown in the resultlist below.<br />
- Show Partials will remove the bosses for which you don't have any pieces. 
(I.e: Does not show anything with availability = 0)<br />
- Show Completable will only display pocket bosses where you have the
sufficient patterns available to make a complete ABCD assembly. (I.e: It will 
limit the result list to bosses where the availability is >= 1.0)<br />
- Checking on the "Exclude ABCD Assemblies" will eliminate the already 
assembled patterns from the search.</p>

<p><b>D</b> : This is a progress meter to show you how far the pattern matcher
has come in its search. When this is all filled up, all your search settings 
have been applied to the database.</p>

<p><b>E</b> : This is the resultlist. Here all (or a subset as per your filter
settings, see A, B &amp; C above) pocket bosses are listed, with the calculated
availability. Selecting a pocket boss in the list will bring up a report in the
web browser that shows the location of all your pattern pieces for this specific
pocket boss.</p>

<?php include("shared_end.php"); ?>