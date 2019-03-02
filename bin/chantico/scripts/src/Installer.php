<?php
  namespace Chantico;

  use Composer\Script\Event;
  use Wadapi\System\ComposerScripts;

  class Installer extends ComposerScripts{
      public static function setup(Event $event){
        $environments = ["development","production"];
        foreach($environments as $environment){
          $response = "";
          while(!in_array(strtolower($response),["y","n","yes","no"])){
            $response = readline("Would you like to configure the $environment environment now [Y/n]? \n");
          }

          if(in_array(strtolower($response),["y","yes"])){
            self::configureChantico($environment, $event);
            self::configureEnvironment($environment, $event);
          }
        }

        self::generateUtilityKey($event);
        self::generateSalt($event);
      }

      private static function configureChantico($environment, Event $event){
        $environmentFile = $event->getComposer()->getConfig()->get("vendor-dir")."/../environments.json";
        $environments = json_decode(file_get_contents($environmentFile),true);

        //Configure environmental database settings
        $url = readline("Enter $environment root url of the managed Wadapi instance []: ");
        $key = readline("Enter $environment authenticator key for '$url' []: ");
        $secret = readline("Enter $environment authenticator secret for '$url' []: ");

        $environments[$environment]["wadapi-instance"] = [
          "url"=>$url?$url:"",
          "key"=>$key?$key:"",
          "secret"=>$secret?$secret:""
        ];

        file_put_contents($environmentFile,json_encode($environments,JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES));
      }

      private static function generateSalt(Event $event){
        $settingsFile = $event->getComposer()->getConfig()->get("vendor-dir")."/../settings.json";
        $settings = json_decode(file_get_contents($settingsFile),true);

        $characters = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        $salt = "";
        $saltLength = 22;

        for($i=0; $i < $saltLength; $i++){
          $salt .= $characters[mt_rand(0, strlen($characters) - 1)];
        }

        $settings["encryption"] = [
          "salt" => $salt
        ];

        file_put_contents($settingsFile,json_encode($settings,JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES));
        echo "Successfully Generated Encryption Salt: [$salt]\n";
      }
  }
?>
