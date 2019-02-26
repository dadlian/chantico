#!/usr/bin/php

<?php
  require 'vendor/autoload.php';
  define('PROJECT_PATH', dirname(__FILE__));

  use Wadapi\Persistence\DatabaseAdministrator;
  use Wadapi\System\Janitor;

  $apiId = "14915827114381";
  DatabaseAdministrator::buildConnection("mywadapi.com","login_main","NLnai%IU7EEv","login_main","chantico");

  $resources = array();
  $apiTokens = array();
  $users = array();
  $userOptions = array();

  foreach(DatabaseAdministrator::execute("SELECT * FROM ManagedAccess WHERE api = '$apiId'") as $managedAccess){
    $apiToken = DatabaseAdministrator::execute("SELECT * FROM APIToken WHERE accessKey = MD5('{$managedAccess['username']}')")[0];
    $user = $managedAccess;

    $resources[] = DatabaseAdministrator::execute("SELECT * FROM Resource WHERE id = {$apiToken['id']}")[0];
    $resources[] = DatabaseAdministrator::execute("SELECT * FROM Resource WHERE id = {$user['id']}")[0];

    $apiTokens[] = $apiToken;
    $users[] = $user;

    $userOptions[] = DatabaseAdministrator::execute("SELECT * FROM ManagedAccessOptions WHERE managedaccess = '{$user['id']}'");
  }

  DatabaseAdministrator::buildConnection("localhost","acym_auth","e(6TP:x87AZ&","acym_auth","chantico");

  foreach($resources as $resource){
    DatabaseAdministrator::execute("INSERT INTO Resource VALUES('{$resource['id']}','{$resource['created']}','{$resource['modified']}')");
  }

  foreach($apiTokens as $apiToken){
    DatabaseAdministrator::execute("INSERT INTO APIToken VALUES('{$apiToken['id']}','{$apiToken['created']}','{$apiToken['modified']}','{$apiToken['role']}','".($apiToken['expires']?$apiToken['expires']:0)."','{$apiToken['accessKey']}','{$apiToken['accessSecret']}','{$apiToken['refreshSecret']}','".($apiToken['disabled']?$apiToken['disabled']:0)."',NULL)");
  }

  foreach($users as $user){
    DatabaseAdministrator::execute("INSERT INTO ManagedAccess VALUES('{$user['id']}','{$user['created']}','{$user['modified']}','{$user['username']}','{$user['authentication']}','{$user['accessKey']}','{$user['accessSecret']}','{$user['refreshSecret']}','{$user['accessEndpoint']}','{$user['expires']}','{$user['multiSession']}')");
  }

  foreach($userOptions as $userOption){
    foreach($userOption as $option){
      $option['value'] = preg_replace("/'/","",$option['value']);
      DatabaseAdministrator::execute("INSERT INTO ManagedAccessOptions VALUES('{$option['managedaccess']}','{$option['name']}','{$option['value']}')");
    }
  }

  Janitor::cleanup();
?>
