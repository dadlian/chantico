<?php
	use Wadapi\Http\ResponseHandler;
	use Wadapi\Http\PagedResourceController;
	use Wadapi\Persistence\SQLGateway;
	use Wadapi\Persistence\Searcher;
	use Wadapi\Persistence\Criterion;
	use Wadapi\System\SettingsManager;

	abstract class ChanticoController extends PagedResourceController{
		public function execute(){
			$sqlGateway = new SQLGateway();
			$searcher = new Searcher();
			$searcher->addCriterion("title",Criterion::EQUAL,"blacklist");
			$blacklist = $sqlGateway->findUnique("IPList",$searcher);

			$searcher = new Searcher();
			$searcher->clearCriteria();
			$searcher->addCriterion("title",Criterion::EQUAL,"whitelist");
			$whitelist = $sqlGateway->findUnique("IPList",$searcher);

			if($blacklist && in_array($_SERVER['REMOTE_ADDR'],$blacklist->getEntries())){
				ResponseHandler::forbidden("The originating IP address is not permitted to make requests.");
			}

			if($whitelist && $whitelist->getEntries() && !in_array($_SERVER['REMOTE_ADDR'],$whitelist->getEntries())){
				ResponseHandler::forbidden("The originating IP address is not permitted to make requests.");
			}

			parent::execute();
		}

		protected function getInvalidQueryParameters(){
				$invalidQueryParameters = array();
				return $invalidQueryParameters;
		}

		abstract protected function getRecordCount();
		abstract protected function buildPageEntries($start,$count);

		//Utility Methods
		protected function sendInstanceGet($endpoint,$queryParameters=array(),$alternateKey="",$alternateSecret=""){
			return $this->_sendInstanceRequest($endpoint,"GET",$queryParameters,$alternateKey="",$alternateSecret="");
		}

		protected function sendInstancePost($endpoint,$payload=array(),$alternateKey="",$alternateSecret=""){
			return $this->_sendInstanceRequest($endpoint,"POST",array(),$payload,$alternateKey="",$alternateSecret="");
		}

		protected function sendInstancePut($endpoint,$payload=array(),$alternateKey="",$alternateSecret=""){
			return $this->_sendInstanceRequest($endpoint,"PUT",array(),$payload,$alternateKey="",$alternateSecret="");
		}

		protected function sendInstanceDelete($endpoint,$payload=array(),$alternateKey="",$alternateSecret=""){
			return $this->_sendInstanceRequest($endpoint,"DELETE",array(),$payload,$alternateKey="",$alternateSecret="");
		}

		private function _sendInstanceRequest($endpoint,$method,$queryParameters=array(),$payload=array(),$alternateKey="",$alternateSecret=""){
			$instanceUrl = SettingsManager::getSetting("wadapi-instance","url");
			$key = $alternateKey?$alternateKey:SettingsManager::getSetting("wadapi-instance","key");
			$secret = $alternateSecret?$alternateSecret:SettingsManager::getSetting("wadapi-instance","secret");

			$requestHeaders = [
				"Authorization: Basic ".base64_encode("$key:$secret"),
				"Content-Type: application/json"
			];

			$queryString = "";
			if($queryParameters && is_array($queryParameters)){
				$queryString = "?".http_build_query($queryParameters);
			}

			switch($method){
				case "POST":
					$requestOptions[CURLOPT_POST] = true;
					break;
				case "PUT":
				case "DELETE":
					$requestOptions[CURLOPT_RETURNTRANSFER] = 1;
					$requestOptions[CURLOPT_HTTPHEADER] = $requestHeaders;
					$requestOptions[CURLOPT_HEADER] = true;

					$consistencyCurl = curl_init("{$instanceUrl}{$endpoint}{$queryString}");
					curl_setopt_array($consistencyCurl,$requestOptions);
					$response = curl_exec($consistencyCurl);
  				$headerString = substr($response, 0, curl_getinfo($consistencyCurl, CURLINFO_HEADER_SIZE));
					curl_close($consistencyCurl);

					$headers = array();
					foreach(array_slice(explode("\r\n", trim($headerString)),1) as $header){
						$headerParts = explode(": ",$header);
						$headers[$headerParts[0]] = trim($headerParts[1]);
					}

					$requestHeaders[] = "If-Match: {$headers['ETag']}";
					$requestHeaders[] = "If-Unmodified-Since: {$headers['Last-Modified']}";

					$requestOptions[CURLOPT_CUSTOMREQUEST] = $method;
					break;
				default:
					break;
			}

			if(in_array($method,["POST","PUT"])){
				$requestOptions[CURLOPT_POSTFIELDS] = json_encode($payload);
				$requestHeaders[] = "Content-Length: ".strlen($requestOptions[CURLOPT_POSTFIELDS]);
			}

			$requestOptions[CURLOPT_RETURNTRANSFER] = 1;
			$requestOptions[CURLOPT_HTTPHEADER] = $requestHeaders;

			$ch = curl_init("{$instanceUrl}{$endpoint}{$queryString}");
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
