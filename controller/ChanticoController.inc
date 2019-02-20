<?php
	use Wadapi\Http\ResponseHandler;
	use Wadapi\Http\PagedResourceController;

	abstract class ChanticoController extends PagedResourceController{
		public function execute(){
			$blacklist = $this->getResourceObject("IPList","title","blacklist");
			$whitelist = $this->getResourceObject("IPList","title","whitelist");

			if($blacklist && in_array($_SERVER['REMOTE_ADDR'],$blacklist->getEntries())){
				ResponseHandler::forbidden("The originating IP address is not permitted to make requests.");
			}

			if($whitelist && $whitelist->getEntries() && !in_array($_SERVER['REMOTE_ADDR'],$whitelist->getEntries())){
				ResponseHandler::forbidden("The originating IP address is not permitted to make requests.");
			}

			parent::execute();
		}

		abstract protected function getInvalidQueryParameters();
		abstract protected function getRecordCount();
		abstract protected function buildPageEntries($start,$count);

		//Utility Methods
		protected sendInstanceGet($endpoint,$queryParameters=array(),$alternateKey="",$alternateSecret=""){
			$this->_sendInstanceRequest($endpoint,"GET",$queryParameters);
		}

		protected sendInstancePost($endpoint,$payload=array(),$alternateKey="",$alternateSecret=""){
			$this->_sendInstanceRequest($endpoint,"POST",array(),$payload);
		}

		protected sendInstancePut($endpoint,$payload=array(),$alternateKey="",$alternateSecret=""){
			$this->_sendInstanceRequest($endpoint,"PUT",array(),$payload);
		}

		protected sendInstanceDelete($endpoint,$payload=array(),$alternateKey="",$alternateSecret=""){
			$this->_sendInstanceRequest($endpoint,"DELETE",array(),$payload);
		}

		private _sendInstanceRequest($endpoint,$method,$queryParameters=array(),$payload=array(),$alternateKey="",$alternateSecret=""){
			$instanceUrl = SettingsManager::getSetting("wadapi-instance","url");
			$instanceKey = SettingsManager::getSetting("wadapi-instance","key");
			$instanceSecret = SettingsManager::getSetting("wadapi-instance","secret");

			$requestHeaders = [
				"Authorization: Basic ".base64_encode("$instanceKey:$instanceSecret"),
				"Content-Type: application/json"
			];

			$queryString = "";
			if($queryParameters && is_array($queryParameters)){
				$queryString = "?".http_build_query($queryParameters);
			}

			switch($method){
				case "POST":
					$requestOptions["CURLOPT_POST"] = true;
					break;
				case "PUT":
				case "DELETE":
					$requestOptions["CURLOPT_RETURNTRANSFER"] = 1;
					$requestOptions["CURLOPT_URL"] = "{$instanceUrl}{$endpoint}{$queryString}";
					$requestOptions["CURLOPT_HTTPHEADER"] = $requestHeaders;
					$requestOptions["CURLOPT_HEADER"] = true;
					$requestOptions["CURLOPT_NOBODY"] = true;

					$consistencyCurl = curl_init("{$instanceUrl}{$endpoint}{$queryString}");
					curl_setopt_array($consistencyCurl,$requestOptions);
  				$headerString = substr($response, 0, curl_getinfo($consistencyCurl, CURLINFO_HEADER_SIZE));
					curl_close($consistencyCurl);

					$headers = array();
					foreach(explode("\r\n", $headerString) as $header){
						$headerParts = explode(":",$header);
						$headers[$headerParts[0]] = trim($headerParts[1]);
					}

					$requestHeaders[] = "If-Match: {$headers['ETag']}";
					$requestHeaders[] = "If-Unmodified-Since: {$headers['Last-Modified']}";

					$requestOptions["CURLOPT_CUSTOMREQUEST"] = $method;
					break;
				default:
					break;
			}

			if(in_array($method,["POST","PUT"]) && $payload && is_array($payload)){
				$requestOptions["CURLOPT_POSTFIELDS"] = json_encode($payload);
				$requestHeaders[] = "Content-Length: ".strlen($requestOptions["CURLOPT_POSTFIELDS"]);
			}

			$requestOptions["CURLOPT_RETURNTRANSFER"] = 1;
			$requestOptions["CURLOPT_URL"] = "{$instanceUrl}{$endpoint}{$queryString}";
			$requestOptions["CURLOPT_HTTPHEADER"] = $requestHeaders;

			$ch = curl_init();
			curl_setopt_array($ch,$requestOptions);
			$response = curl_exec($ch);
			$responseCode = curl_getinfo($ch, CURLINFO_RESPONSE_CODE);

			curl_close($ch);

			return [
				"code"=>$responseCode,
				"body"=>json_decode($response,true)
			];
		}
	}
?>
