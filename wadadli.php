<?php
	$settings = dirname(__FILE__)."/api/settings.xml";
        require_once(dirname(__FILE__)."/../core/lib/includes.inc");
       
	//Transform access key token to lowercase
	$authorisation = RequestHandler::getHeader("Authorization");
	$authorisationParts = preg_split("/ /",$authorisation);
	$authorisationTokens = preg_split("/:/",base64_decode($authorisationParts[1]));
	if(sizeof($authorisationTokens) > 1){
		RequestHandler::changeHeader("Authorization","Basic ".base64_encode(strtolower($authorisationTokens[0]).":".$authorisationTokens[1]));
	}

	//Dispatch Request
        $activeDispatcher = SettingsManager::getSetting("install","dispatcher");
        $activeDispatcher::dispatchRequest();
?>
