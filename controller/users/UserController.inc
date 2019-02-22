<?php
	use Wadapi\Http\RequestHandler;
	use Wadapi\Http\ResponseHandler;
	use Wadapi\System\SettingsManager;
	use Wadapi\Persistence\DatabaseAdministrator;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\Persistence\Searcher;
	use Wadapi\Persistence\Sorter;
	use Wadapi\Persistence\Criterion;

	abstract class UserController extends ChanticoController{
		public function execute(){
			if(RequestHandler::getAuthenticatedToken()->getRole() == "user"){
				$user = $this->getResourceObject("ManagedAccess","id",$this->viewFromArguments("user"));
				$authorisation = RequestHandler::getAuthorisation();

				if($user && !($user->getUsername() == strtolower($authorisation["key"]) && $user->getAuthentication() == md5($authorisation["secret"]))){
					ResponseHandler::forbidden("The provided tokens do not have permission to perform this action.");
				}
			}

			parent::execute();
		}

		protected function isInvalid(){
			$invalidArguments = array();

			if(!is_string($this->getFromContent("username")) || preg_match("/\s/",$this->getFromContent("username"))){
				$invalidArguments[] = "username";
			}

			if(!is_string($this->getFromContent("authentication"))){
				$invalidArguments[] = "authentication";
			}

			if($this->getFromContent("role") && !is_string($this->getFromContent("role"))){
				$invalidArguments[] = "role";
			}

			if(!is_bool($this->getFromContent("multi-session",false))){
				$invalidArguments[] = "multi-session";
			}

			if(!is_array($this->getFromContent("options",array()))){
				$invalidArguments[] = "options";
			}else{
				foreach($this->getFromContent("options",array()) as $option){
					if(!is_string($option) && !is_array($option)){
						$invalidArguments[] = "options";
						break;
					}
				}
			}
			return $invalidArguments;
		}

		protected function getInvalidQueryParameters(){
			$invalidQueryParameters = array();

			$usernames = RequestHandler::getQueryParameter("usernames");
			if(!is_string($usernames)){
				$invalidQueryParameters[] = "usernames";
			}

			$options = RequestHandler::getQueryParameter("options");
			if($options && !preg_match("/^(,?[\w\-]+~[\w:\/\-\.@]+)+$/",$options)){
				$invalidQueryParameters[] = "options";
			}

			$access = RequestHandler::getQueryParameter("access");
			if(!is_string($access)){
				$invalidQueryParameters[] = "access";
			}

			return $invalidQueryParameters;
		}

		protected function getRecordCount(){
			$whereParams = array();
			$usernames = RequestHandler::getQueryParameter("usernames");
			$options = RequestHandler::getQueryParameter("options");
			$access = RequestHandler::getQueryParameter("access");

			if($usernames){
				$usernameConditions = array();
				foreach(explode(",",$usernames) as $username){
					$username = strtolower($username);
					$usernameConditions[] = "username LIKE '$username'";
				}

				if($usernameConditions){
					$whereParams[] =  "(".implode(" OR ",$usernameConditions).")";
				}
			}

			if($options){
				$optionsList = explode(",",$options);
				$optionConditions = array();
				foreach($optionsList as $option){
					$parameters = preg_split("/~/",$option);
					$jsonParameter = addslashes(json_encode($parameters[1]));
					$optionConditions[] = "(name = '{$parameters[0]}' AND (value = '{$parameters[1]}' OR value = \"$jsonParameter\"))";
				}

				if($optionConditions){
					$whereParams[] = "id IN (SELECT managedaccess FROM 	ManagedAccessOptions WHERE (".implode(" OR ",$optionConditions)."))";
				}
			}

			if($access){
				$whereParams[] = "accessEndpoint = '$access'";
			}

			$whereClause = $whereParams?("WHERE ".implode(" AND ",$whereParams)):"";
			$countResult = DatabaseAdministrator::execute("SELECT COUNT(id) as user FROM ManagedAccess $whereClause");
			$count = $countResult[0]["user"];

			return $count;
		}

		protected function buildPageEntries($start,$count){
			$whereParams = array();
			$usernames = RequestHandler::getQueryParameter("usernames");
			$options = RequestHandler::getQueryParameter("options");
			$access = RequestHandler::getQueryParameter("access");

			if($usernames){
				$usernameConditions = array();
				foreach(explode(",",$usernames) as $username){
					$username = strtolower($username);
					$usernameConditions[] = "username LIKE '$username'";
				}

				if($usernameConditions){
					$whereParams[] =  "(".implode(" OR ",$usernameConditions).")";
				}
			}

			if($options){
				$optionsList = explode(",",$options);
				$optionConditions = array();
				foreach($optionsList as $option){
					$parameters = preg_split("/~/",$option);
					$jsonParameter = addslashes(json_encode($parameters[1]));
					$optionConditions[] = "(name = '{$parameters[0]}' AND (value = '{$parameters[1]}' OR value = \"$jsonParameter\"))";
				}

				if($optionConditions){
					$whereParams[] = "id IN (SELECT managedaccess FROM ManagedAccessOptions WHERE (".implode(" OR ",$optionConditions)."))";
				}
			}

			if($access){
				$whereParams[] = "accessEndpoint = '$access'";
			}

			$whereClause = $whereParams?("WHERE ".implode(" AND ",$whereParams)):"";
			$results = DatabaseAdministrator::execute("SELECT id FROM ManagedAccess $whereClause");

			$users = array();
			foreach($results as $user){
				$users[] = $user["id"];
			}

			$payload = array();
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("id",Criterion::EQUAL,$users);
			foreach($sqlGateway->find("ManagedAccess",$searcher,new Sorter(),$count,$start,false) as $user){
				$payload[] = $this->assemblePayload($user);
			}

			return $payload;
		}

		protected function assemblePayload($user){
			//Load User Options from API
			$realOptions = array();
			foreach($user->getOptions() as $option => $value){
				if($option != "role"){
					$realOptions[$option] = $value;
				}
			}

			$optionsInitialised = sizeof($realOptions) > 0;
			foreach($realOptions as $option){
				if(!$option){
					$optionsInitialised = false;
					break;
				}
			}

			if(!$optionsInitialised){
				$response = $this->sendInstanceGet($user->getAccessEndpoint());
				$options = array();
				if($response['code'] == "200"){
					foreach($response['body'] as $option => $value){
						$options[$option] = json_encode($value);
					}
				}

				$user->setOptions($options);
				$sqlGateway = new SQLGateway();
				$sqlGateway->save($user);
			}

			$options = array();
			foreach($user->getOptions() as $option => $value){
				$optionValue = json_decode($value);
				if(json_last_error() != JSON_ERROR_NONE){
					$optionValue = $value;
				}

				$options[$option] = $optionValue;
			}

			unset($options["role"]);

			$realRole = json_decode($user->viewFromOptions("role"));
			if(json_last_error() != JSON_ERROR_NONE){
				$realRole = $user->viewFromOptions("role");
			}

			$instanceUrl = SettingsManager::getSetting("wadapi-instance","url");
			$payload = array(
				"self"=>$user->getURI(),
				"username"=>$user->getUsername(),
				"role"=>$realRole,
				"multi-session"=>$user->isMultiSession(),
				"access"=>"$instanceUrl{$user->getAccessEndpoint()}",
				"options"=>$options,
				"tokens"=>"{$user->getURI()}/tokens",
				"resets"=>"{$user->getURI()}/resets",
				"recoveries"=>"{$user->getURI()}/recoveries"
			);

			return $payload;
		}

		protected function encrypt($data,$key){
			$key = md5(password_hash($key,PASSWORD_DEFAULT,array("salt"=>SettingsManager::getSetting("encryption","salt"))));
			$key = pack('H*',$key);

			# create a random IV to use with CBC encoding
			$iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_128, MCRYPT_MODE_CBC);
			$iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);

			# creates a cipher text compatible with AES (Rijndael block size = 128)
			$ciphertext = $iv . mcrypt_encrypt(MCRYPT_RIJNDAEL_128, $key, $data, MCRYPT_MODE_CBC, $iv);
			return base64_encode($ciphertext);
		}

		protected function decrypt($data,$key){
			$key = md5(password_hash($key,PASSWORD_DEFAULT,array("salt"=>SettingsManager::getSetting("encryption","salt"))));
			$key = pack('H*',$key);
			$ciphertext = base64_decode($data);

			# retrieves the IV, iv_size should be created using mcrypt_get_iv_size()
			$iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_128, MCRYPT_MODE_CBC);
			$iv = substr($ciphertext, 0, $iv_size);

			# retrieves the cipher text (everything except the $iv_size in the front)
			$ciphertext = substr($ciphertext, $iv_size);
			return mcrypt_decrypt(MCRYPT_RIJNDAEL_128, $key, $ciphertext, MCRYPT_MODE_CBC, $iv);
		}
	}
?>
