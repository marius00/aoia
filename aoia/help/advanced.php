<?php include("shared_start.php"); ?>

<h1>AO Item Assistant : The Advanced Stuff</h1>

<h2>Override Database Location</h2>
To specify the name and location of your database file, you need to supply the 
application with the option called <i>-db</i>.
<p />
Example: <strong>ItemAssistant.exe -db c:\MyItemAssistant.db</strong>
<p />
You can add this permanently to the shortcut you use to launch AO Item Assistant with.

<h2>Enable Logging</h2>
If you are having problems with the application you can add the "-log" 
parameter when launching it. This will generate a log-file that may give hints
about what the problem might be.
<p />
Example: <strong>ItemAssistant.exe -log</strong>
<p />

<?php include("shared_end.php"); ?>