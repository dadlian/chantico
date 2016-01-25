#!/usr/bin/php
<?php
	$settingsFile = simplexml_load_file(dirname(__FILE__)."/../settings.xml");
	$hostname = $settingsFile->settingslist->settings[1]->values->hostname;
	$database = $settingsFile->settingslist->settings[1]->values->database;
	$username = $settingsFile->settingslist->settings[1]->values->username;
	$password = $settingsFile->settingslist->settings[1]->values->password;
	$conn = new mysqli($hostname,$database,$password,$username);

	$result = $conn->query("SELECT id, modified FROM chantico_PasswordReset WHERE status = 'requested'");
	while($row = $result->fetch_assoc()){
		if($row['modified'] + 1800 < microtime(true)){
			$conn->query("UPDATE chantico_PasswordReset SET status = 'cancelled' WHERE id = {$row['id']}");
		}
	}
	
	$conn->commit();
	$conn->close();
?>