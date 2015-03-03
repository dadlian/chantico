<?php
        require_once(dirname(__FILE__)."/../lib/includes.inc");
        
        //Dispatch Request
        $activeDispatcher = SettingsManager::getSetting("install","dispatcher");
        $activeDispatcher::dispatchRequest();
?>