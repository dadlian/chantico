<?php	
	class UserTests extends APITestCase{
		function testSuccessfulUserCollectionManipulation(){
			/**** Test new user creation ****/
			$username = "tester";
			$password = "tester";
			$role = "consumer";
			
			$this->request->setEndpoint("/apis/14358019264956/users");
			$payload = json_encode(array("username"=>$username,"authentication"=>$password,"role"=>$role));
			$this->request->setBody($payload);
			$response = $this->request->post();
			
			//Test for Valid Status Line
			$this->assertEqual(201,$response->getStatusCode(),"Successful user creation returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("Created",$response->getReason(),"Successful user creation returns the wrong reason: {$response->getReason()}");
			
			//Test for Mandatory Headers
			$datePattern = "/".gmdate("D, d M Y")." [0-2][0-9]:[0-5][0-9]:[0-5][0-9] GMT/";
			$locationPattern = "/http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."/apis/14358019264956/users/"))."[0-9]{14}/";
			
			$this->assertPattern($datePattern,$response->viewFromHeaders("Date"),"Successful user creation responds with a malformed or incorrect Date Header: {$response->viewFromHeaders('Date')}");
			$this->assertEqual(strlen($response->getBody()),$response->viewFromHeaders("Content-Length"),"Successful user creation responds with the incorrect Content-Length: ".$response->viewFromHeaders("Content-Length"));
			$this->assertPattern($locationPattern,$response->viewFromHeaders("Content-Location"),"Successful user creation responds with the incorrect Content-Location: ".$response->viewFromHeaders("Content-Location"));
			
			//Test for Resource Specific Headers
			$this->assertPattern($locationPattern,$response->viewFromHeaders("Location"),"Successful user creation responds with the incorrect Location: ".$response->viewFromHeaders("Location"));

			//Ensure there are no unexpected Headers
			$this->assertEqual(8,sizeof($response->getHeaders(false)),"Successful user creation responds with superfluous headers (8 expected, ".sizeof($response->getHeaders())." given)");
			
			//Test new User Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful user creation does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully created user resource does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users\/[0-9]{14}";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully created user resource includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("username",$responseBody),"A successfully created user resource does not include a username.");
				if(array_key_exists("username",$responseBody)){
					$this->assertEqual($username,$responseBody["username"],"A successfully created user resource includes the wrong username: {$responseBody['username']}");
				}
				
				$this->assertTrue(array_key_exists("role",$responseBody),"A successfully created user resource does not include a role.");
				if(array_key_exists("role",$responseBody)){
					$this->assertEqual($role,$responseBody["role"],"A successfully created user resource includes the wrong role: {$responseBody['role']}");
				}
				
				$this->assertTrue(array_key_exists("tokens",$responseBody),"A successfully created user resource does not include a tokens link.");
				if(array_key_exists("tokens",$responseBody)){
					$tokensPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users\/[0-9]{14}\/tokens";
					$this->assertPattern("/$tokensPattern/",$responseBody["tokens"],"A successfully created user resource includes an invalid tokens link: {$responseBody['tokens']}");
				}
				
				$this->assertTrue(array_key_exists("options",$responseBody),"A successfully created user resource does not include an options array.");
				if(array_key_exists("options",$responseBody)){
					$this->assertEqual(array("profile"=>""),$responseBody["options"],"A successfully created user resource includes an invalid options array: ".implode(",",$responseBody['options']));
				}
			}
			
			/**** Test new user collection retrieval ****/
			
			//Add sample users
			$username = array("tester1","tester2","tester3");
			$password = array("tester","tester","tester");
			$role = array("consumer","consumer","consumer");
			
			$this->request->setEndpoint("/apis/14358019264956/users");
			$this->request->setBody(json_encode(array("username"=>$username[0],"authentication"=>$password[0],"role"=>$role[0],"options"=>array())));
			$this->request->post()->viewFromHeaders("Location");
			
			$this->request->setEndpoint("/apis/14358019264956/users");
			$this->request->setBody(json_encode(array("username"=>$username[1],"authentication"=>$password[1],"role"=>$role[1],"options"=>array())));
			$this->request->post()->viewFromHeaders("Location");
			
			$this->request->setEndpoint("/apis/14358019264956/users");
			$this->request->setBody(json_encode(array("username"=>$username[2],"authentication"=>$password[2],"role"=>$role[2],"options"=>array())));
			$this->request->post()->viewFromHeaders("Location");
			
			//Get full users collection
			$this->request->setEndpoint("/apis/14358019264956/users");
			$this->request->setBody("");
			$response = $this->request->get();
			
			$this->assertTrue($this->validJSON($response->getBody()),"Retrieving user collection does not return valid JSON in the response body.");
			
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved user collection does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved user collection includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("total",$responseBody),"A successfully retrieved user collection does not include the total record count.");
				if(array_key_exists("total",$responseBody)){
					$this->assertEqual(4,$responseBody["total"],"A successfully retrieved user collection includes the wrong total record count: {$responseBody['total']}");
				}
				
				$this->assertTrue(array_key_exists("entries",$responseBody),"A successfully retrieved user collection does not include an array of users.");
				if(array_key_exists("entries",$responseBody)){
					$this->assertEqual(4,sizeof($responseBody["entries"]),"A successfully retrieved user collection includes the incorrect number of users: ".sizeof($responseBody['entries']));

					//Check that contents of random user are valid
					$index = rand(0,2);
					$responseBody = $responseBody['entries'][$index+1];
					
					$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved user resource does not include a self link.");
					if(array_key_exists("self",$responseBody)){
						$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot)."/apis/14358019264956/users/")."[0-9]{14}";
						$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved user resource includes an invalid self link: {$responseBody['self']}");
					}
					
					$this->assertTrue(array_key_exists("username",$responseBody),"A successfully retrieved user resource does not include a username.");
					if(array_key_exists("username",$responseBody)){
						$this->assertEqual($username[$index],$responseBody["username"],"A successfully retrieved user resource includes the wrong username: {$responseBody['username']}");
					}
					
					$this->assertTrue(array_key_exists("role",$responseBody),"A successfully retrieved user resource does not include a role.");
					if(array_key_exists("role",$responseBody)){
						$this->assertEqual($role[$index],$responseBody["role"],"A successfully retrieved user resource includes the wrong role: {$responseBody['role']}");
					}
					
					$this->assertTrue(array_key_exists("tokens",$responseBody),"A successfully retrieved user resource does not include a tokens link.");
					if(array_key_exists("tokens",$responseBody)){
						$tokensPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users\/[0-9]{14}\/tokens";
						$this->assertPattern("/$tokensPattern/",$responseBody["tokens"],"A successfully retrieved user resource includes an invalid tokens link: {$responseBody['tokens']}");
					}
				
					$this->assertTrue(array_key_exists("options",$responseBody),"A successfully retrieved user resource does not include an options array.");
					if(array_key_exists("options",$responseBody)){
						$this->assertEqual(array("profile"=>""),$responseBody["options"],"A successfully retrieved user resource includes an invalid options array: ".implode(",",$responseBody['options']));
					}
				}
			}
			
			//Get paged users collection
			$this->request->setEndpoint("/apis/14358019264956/users");
			$this->request->insertToArguments("page","2");
			$this->request->insertToArguments("records","1");
			$response = $this->request->get();
			
			$this->assertTrue($this->validJSON($response->getBody()),"Retrieving paginated user collection does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved paginated user collection does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users\?page=2&records=1";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved paginated user collection includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("prev",$responseBody),"A successfully retrieved paginated user collection does not include a link to the previous page.");
				if(array_key_exists("prev",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users\?page=1&records=1";
					$this->assertPattern("/$userPattern/",$responseBody["prev"],"A successfully retrieved paginated user collection includes an invalid link to the previous page: {$responseBody['prev']}");
				}
				
				$this->assertTrue(array_key_exists("total",$responseBody),"A successfully retrieved paginated user collection does not include the total record count.");
				if(array_key_exists("total",$responseBody)){
					$this->assertEqual(4,$responseBody["total"],"A successfully retrieved paginated user collection includes the wrong total record count: {$responseBody['total']}");
				}
				
				$this->assertTrue(array_key_exists("entries",$responseBody),"A successfully retrieved paginated user collection does not include an array of users.");
				if(array_key_exists("entries",$responseBody)){
					$this->assertEqual(1,sizeof($responseBody["entries"]),"A successfully retrieved paginated user collection includes the incorrect number of users: ".sizeof($responseBody['entries']));
				}
			}
			
			//Get paged user collection by username
			$this->request->setEndpoint("/apis/14358019264956/users");
			$this->request->clearArguments();
			$this->request->insertToArguments("page","2");
			$this->request->insertToArguments("records","1");
			$this->request->insertToArguments("usernames","{$username[0]},{$username[2]}");
			$response = $this->request->get();
			
			$this->assertTrue($this->validJSON($response->getBody()),"Retrieving paginated user collection by username does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved paginated user collection by username does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users\?page=2&records=1&usernames={$username[0]},{$username[2]}";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved paginated user collection by username includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("prev",$responseBody),"A successfully retrieved paginated user collection does not include a link to the previous page.");
				if(array_key_exists("prev",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users\?page=1&records=1&usernames={$username[0]},{$username[2]}";
					$this->assertPattern("/$userPattern/",$responseBody["prev"],"A successfully retrieved paginated user collection by username includes an invalid link to the previous page: {$responseBody['prev']}");
				}
				
				$this->assertTrue(array_key_exists("total",$responseBody),"A successfully retrieved paginated user collection by username does not include the total record count.");
				if(array_key_exists("total",$responseBody)){
					$this->assertEqual(2,$responseBody["total"],"A successfully retrieved paginated user collection by username includes the wrong total record count: {$responseBody['total']}");
				}
				
				$this->assertTrue(array_key_exists("entries",$responseBody),"A successfully retrieved paginated user collection by username does not include an array of users.");
				if(array_key_exists("entries",$responseBody)){
					$this->assertEqual(1,sizeof($responseBody["entries"]),"A successfully retrieved paginated user collection by username includes the incorrect number of users: ".sizeof($responseBody['entries']));
					
					//Check that contents of the user are valid
					$responseBody = $responseBody['entries'][0];
						
					$this->assertTrue(array_key_exists("username",$responseBody),"A user resource in a paginated user collection by username does not include a username.");
					if(array_key_exists("username",$responseBody)){
						$this->assertEqual($username[2],$responseBody["username"],"A user resource in a paginated user collection by username includes the wrong username: {$responseBody['username']}");
					}
				}
			}
		}
		
		function testUserCollectionManipulationHandlesUnsupportedRequests(){
			/**** Test unsupported methods ****/
			$this->request->setEndpoint("/apis/14358019264956/users");
			
			$response = $this->request->put();
			$this->assertEqual(405,$response->getStatusCode(),"/apis/{api_id}/users endpoint returns incorrect status code for unsupported method PUT: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/apis/{api_id}/users endpoint returns incorrect reason for unsupported method PUT: {$response->getReason()}");
			$this->validateErrorMessage($response,"/apis/14358019264956/users does not support the PUT method.","Making an unsupported PUT request to the /apis/{api_id}/users endpoint returns an invalid message");
			
			$response = $this->request->delete();
			$this->assertEqual(405,$response->getStatusCode(),"/apis/{api_id}/users endpoint returns incorrect status code for unsupported method DELETE: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/apis/{api_id}/users endpoint returns incorrect reason for unsupported method DELETE: {$response->getReason()}");
			$this->validateErrorMessage($response,"/apis/14358019264956/users does not support the DELETE method.","Making an unsupported DELETE request to the /apis/{api_id}/users endpoint returns an invalid message");
		}
		
		function testUserCollectionManipulationHandlesMissingOrInvalidQueryParameters(){
			$this->request->setEndpoint("/apis/14358019264956/users");
			
			/**** Test Invalid Query Parameters ****/
			$this->request->insertToArguments("page","page");
			$this->request->insertToArguments("records","records");
			$response = $this->request->get();
			$this->assertEqual(400,$response->getStatusCode(),"/apis/{api_id}/users endpoint returns incorrect status code for invalid query parameters: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/apis/{api_id}/users endpoint returns incorrect reason for invalid query parameters: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following query parameters have invalid values: page, records.","Supplying invalid query parameter values to the /apis/{api_id}/users endpoint returns an invalid message");
		}
		
		function testUserCollectionManipulationHandlesMissingOrInvalidArguments(){
			$this->request->setEndpoint("/apis/14358019264956/users");
			
			
			/**** Test Non-JSON Request Body ****/
			$this->request->setBody("Invalid JSON");
			$response = $this->request->post();
			$this->assertEqual(400,$response->getStatusCode(),"/apis/{api_id}/users endpoint returns incorrect status code for invalid JSON request body: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/apis/{api_id}/users endpoint returns incorrect reason for invalid JSON request body: {$response->getReason()}");
			$this->validateErrorMessage($response,"Request arguments must be supplied using valid JSON.","POSTing to the /apis/{api_id}/users endpoint with a non-JSON body returns an invalid message");
			
			
			/**** Test Missing Required Arguments ****/
			$this->request->setBody(json_encode(array()));
			$response = $this->request->post();
			$this->assertEqual(400,$response->getStatusCode(),"/apis/{api_id}/users endpoint returns incorrect status code for missing required arguments: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/apis/{api_id}/users endpoint returns incorrect reason for missing required arguments: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following arguments are required, but have not been supplied: username, authentication, role.","Failing to supply required arguments to the /apis/{api_id}/users endpoint returns an invalid message");
			
			
			/**** Test Invalid Arguments ****/
			$payload = json_encode(array("username"=>array("user"),"authentication"=>array(),"role"=>"consumer","options"=>"invalid options"));
			$this->request->setBody($payload);
			$response = $this->request->post();
			$this->assertEqual(400,$response->getStatusCode(),"/apis/{api_id}/users endpoint returns incorrect status code for invalid arguments: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/apis/{api_id}/users endpoint returns incorrect reason for invalid arguments: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following arguments have invalid values: username, authentication, options.","Supplying invalid argument values to the /apis/{api_id}/users endpoint returns an invalid message");
			
			$payload = json_encode(array("username"=>"validuser","authentication"=>"a string password","role"=>"root","options"=>array()));
			$this->request->setBody($payload);
			$response = $this->request->post();
			$this->assertEqual(400,$response->getStatusCode(),"/apis/{api_id}/users endpoint returns incorrect status code for invalid role: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/apis/{api_id}/users endpoint returns incorrect reason for invalid role: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following directives have invalid values: role.","Supplying invalid role value to the /apis/{api_id}/users endpoint returns an invalid message");
			
			/**** Test Duplicate Argument Values ****/
			$this->request->setBody(json_encode(array("username"=>"orig_user","authentication"=>"test","role"=>"consumer")));
			$this->request->post();
			
			//Make Duplicate Post
			$response = $this->request->post();
			$this->assertEqual(409,$response->getStatusCode(),"Creating a duplicate user resource returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Conflict",$response->getReason(),"Creating a duplicate user resource returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"A user identified by 'orig_user' already exists.","Creating a duplicate user resource returns an invalid message");
		}
		
		function testUserCollectionManipulationHandlesUnauthorisedRequests(){
			$this->request->setEndpoint("/apis/14358019264956/users");

			/**** Test Unauthorised Request ****/
			$this->request->takeFromHeaders("Authorization");
			$response = $this->request->post();
			$this->assertEqual(401,$response->getStatusCode(),"Unauthorised request to /apis/{api_id}/users endpoint returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Authorization Required",$response->getReason(),"Unauthorised request to /apis/{api_id}/users endpoint returns incorrect reason: {$response->getReason()}");
			$this->assertEqual("Basic",$response->viewFromHeaders("WWW-Authenticate"),"Unauthorised request to /apis/{api_id}/users endpoint does not return correct WWW-Authenticate header: {$response->viewFromHeaders('WWW-Authenticate')}");
			$this->validateErrorMessage($response,"Please use Basic Authentication to authorise this request.","Sending an Unauthorised request to the /apis/{api_id}/users endpoint returns an invalid message");
			
			
			/**** Test Incorrectly Authorised Request ****/
			$this->request->authorise("baduser","badpass");
			$response = $this->request->post();
			$this->assertEqual(403,$response->getStatusCode(),"Accessing /apis/{api_id}/users endpoint with unauthorised credentials returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Accessing /apis/{api_id}/users endpoint with unauthorised credentials returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The provided tokens do not have permission to perform this action.","Accessing /apis/{api_id}/users endpoint with unauthorised credentials returns an invalid message");
		}
		
		function testUserCollectionManipulationNegotiatesContent(){
			$this->request->setEndpoint("/apis/14358019264956/users");
			
			
			/**** Test that correct content types are returned by default ****/
			$this->request->setBody(json_encode(array("username"=>"next_user0","authentication"=>"test","role"=>"consumer")));
			$response = $this->request->post();
			
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user creation request did not return the correct content representation or charset by default.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user creation request did not return the correct content language by default.");
			
			
			/**** Test that supported content types return the correct representation ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/users");
			$this->request->addPreferedContentType("application/json");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("en");
			$this->request->setBody(json_encode(array("username"=>"next_user1","authentication"=>"test","role"=>"consumer")));
			$response = $this->request->post();
			
			$this->assertEqual(201,$response->getStatusCode(),"A successful user creation request for a supported content type did not return a sucessful status code.");
			$this->assertEqual("Created",$response->getReason(),"A successful user creation request for a supported content type did not return a sucessful reason.");
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user creation request for a supported content representation and charset were not honoured.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user creation request for a supported content language was not honoured.");
			
			
			/**** Test that unsupported content types return an error ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/users");
			$this->request->addPreferedContentType("application/json");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("ru");
			$this->request->setBody(json_encode(array("username"=>"next_user2","authentication"=>"test","role"=>"consumer")));
			$response = $this->request->post();
			
			$this->assertEqual(406,$response->getStatusCode(),"Requesting unsupported content types from the /apis/{api_id}/users endpoint returns an incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Acceptable",$response->getReason(),"Requesting unsupported content types from the /apis/{api_id}/users endpoint returns an incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested end-point does not support languages: ru.","Requesting unsupported content types from the /apis/{api_id}/users endpoint returns an incorrect message");
		}
		
		function testRetrievalOfUndefinedUserCollectionPages(){
			$this->request->setEndpoint("/apis/14358019264956/users");
			
			/**** Test requesting a non-existant collection page ****/
			$this->request->insertToArguments("page","12");
			$this->request->insertToArguments("records","3");
			$response = $this->request->get();
			
			$this->assertEqual(404,$response->getStatusCode(),"Accessing non-existant pages of the /apis/{api_id}/users endpoint returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Found",$response->getReason(),"Accessing non-existant pages of the /apis/{api_id}/users endpoint returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The specified page does not exist for the given records per page.","Accessing non-existant pages of the /apis/{api_id}/users endpoint returns an invalid message");
		}
		
		
		function testSuccessfulUserManipulation(){
			//Create a new user resource
			$username = "tester13";
			$password = "tester";
			$role = "consumer";
			
			$this->request->setEndpoint("/apis/14358019264956/users");
			$payload = json_encode(array("username"=>$username,"authentication"=>$password,"role"=>$role));
			$this->request->setBody($payload);
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			/**** Test User Resource is correctly retrieved ****/
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			//Test for Valid Status Line
			$this->assertEqual(200,$response->getStatusCode(),"Successful user retrieval returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("OK",$response->getReason(),"Successful user retrieval returns the wrong reason: {$response->getReason()}");
			
			//Test for Mandatory Headers
			$datePattern = "/".gmdate("D, d M Y")." [0-2][0-9]:[0-5][0-9]:[0-5][0-9] GMT/";
			$locationPattern = "/http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot).$user)."/";
			
			$this->assertPattern($datePattern,$response->viewFromHeaders("Date"),"Successful user retrieval responds with a malformed or incorrect Date Header: {$response->viewFromHeaders('Date')}");
			$this->assertEqual(strlen($response->getBody()),$response->viewFromHeaders("Content-Length"),"Successful user retrieval responds with the incorrect Content-Length: ".$response->viewFromHeaders("Content-Length"));
			$this->assertPattern($locationPattern,$response->viewFromHeaders("Content-Location"),"Successful user retrieval responds with the incorrect Content-Location: ".$response->viewFromHeaders("Content-Location"));
			$this->assertPattern($datePattern,$response->viewFromHeaders("Last-Modified"),"Successful user retrieval responds with a missing, malformed or incorrect Last-Modified Header: {$response->viewFromHeaders('Last-Modified')}");
			$this->assertPattern("/^[a-f0-9]{32}$/",$response->viewFromHeaders("ETag"),"Successful user retrieval responds with a missing or malformed ETag: {$response->viewFromHeaders('Last-Modified')}");
						
			//Ensure there are no unexpected Headers
			$this->assertEqual(9,sizeof($response->getHeaders(false)),"Successful user retrieval responds with superfluous headers (9 expected, ".sizeof($response->getHeaders())." given)");
						
			//Test Retrieved User Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful user retrieval does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved user resource does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users\/[0-9]{14}";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved user resource includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("username",$responseBody),"A successfully retrieved user resource does not include a username.");
				if(array_key_exists("username",$responseBody)){
					$this->assertEqual($username,$responseBody["username"],"A successfully retrieved user resource includes the wrong username: {$responseBody['username']}");
				}
				
				$this->assertTrue(array_key_exists("role",$responseBody),"A successfully retrieved user resource does not include a role.");
				if(array_key_exists("role",$responseBody)){
					$this->assertEqual($role,$responseBody["role"],"A successfully retrieved user resource includes the wrong role: {$responseBody['role']}");
				}
				
				$this->assertTrue(array_key_exists("tokens",$responseBody),"A successfully retrieved user resource does not include a tokens link.");
				if(array_key_exists("tokens",$responseBody)){
					$tokensPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users\/[0-9]{14}\/tokens";
					$this->assertPattern("/$tokensPattern/",$responseBody["tokens"],"A successfully retrieved user resource includes an invalid tokens link: {$responseBody['tokens']}");
				}
				
				$this->assertTrue(array_key_exists("options",$responseBody),"A successfully retrieved user resource does not include an options array.");
				if(array_key_exists("options",$responseBody)){
					$this->assertEqual(array("profile"=>""),$responseBody["options"],"A successfully retrieved user resource includes an invalid options array: ".implode(",",$responseBody['options']));
				}
			}
			
			
			/**** Test User Resource is correctly modified ****/
			$newUsername = "tester4"; $newPassword = "tester"; $newRole = "consumer";
			$this->request->setEndpoint($user);
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode(array("username"=>$newUsername,"authentication"=>$newPassword,"role"=>$newRole)));
			$this->request->authorise($username,$password);
			$response = $this->request->put();
			
			//Test for Valid Status Line
			$this->assertEqual(200,$response->getStatusCode(),"Successful user modification returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("OK",$response->getReason(),"Successful user modification returns the wrong reason: {$response->getReason()}");
			
			//Test for Modified User Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful user modification does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully modified user resource does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users\/[0-9]{14}";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully modified user resource includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("username",$responseBody),"A successfully modified user resource does not include a username.");
				if(array_key_exists("username",$responseBody)){
					$this->assertEqual($newUsername,$responseBody["username"],"A successfully modified user resource includes the wrong username: {$responseBody['username']}");
				}
				
				$this->assertTrue(array_key_exists("role",$responseBody),"A successfully modified user resource does not include a role.");
				if(array_key_exists("role",$responseBody)){
					$this->assertEqual($newRole,$responseBody["role"],"A successfully modified user resource includes the wrong role: {$responseBody['role']}");
				}
				
				$this->assertTrue(array_key_exists("tokens",$responseBody),"A successfully modified user resource does not include a tokens link.");
				if(array_key_exists("tokens",$responseBody)){
					$tokensPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users\/[0-9]{14}\/tokens";
					$this->assertPattern("/$tokensPattern/",$responseBody["tokens"],"A successfully modified user resource includes an invalid tokens link: {$responseBody['tokens']}");
				}
				
				$this->assertTrue(array_key_exists("options",$responseBody),"A successfully modified user resource does not include an options array.");
				if(array_key_exists("options",$responseBody)){
					$this->assertEqual(array("profile"=>""),$responseBody["options"],"A successfully modified user resource includes an invalid options array: ".implode(",",$responseBody['options']));
				}
			}
			
			//Test that modifications were saved
			$this->request->setEndpoint($user);
			$this->request->setBody("");
			$this->request->authorise($newUsername,$newPassword);
			$response = $this->request->get();
			
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(), true);
				
				$this->assertTrue(array_key_exists("username",$responseBody),"A successfully modified user resource does not save a new username.");
				if(array_key_exists("username",$responseBody)){
					$this->assertEqual($newUsername,$responseBody["username"],"A successfully modified user resource saves a new wrong username: {$responseBody['username']}");
				}
				
				$this->assertTrue(array_key_exists("role",$responseBody),"A successfully modified user resource does not save a new role.");
				if(array_key_exists("role",$responseBody)){
					$this->assertEqual($newRole,$responseBody["role"],"A successfully modified user resource saves the wrong role: {$responseBody['role']}");
				}
			}
			
			
			/**** Test user resource is successfully removed ****/
			$this->request->setEndpoint($user);
			$this->request->authorise($newUsername,$newPassword);
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$response = $this->request->delete();
			
			//Test for Valid Status Line
			$this->assertEqual(200,$response->getStatusCode(),"Successful user deletion returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("OK",$response->getReason(),"Successful user deletion returns the wrong reason: {$response->getReason()}");
			
			//Test for Removed User Response Content
			$this->validateErrorMessage($response,"User: $user, has been removed.","Removing a /user resource returns an incorrect message");
			
			//Test that user has been removed from the system
			$this->initialiseRequest();
			$this->request->setEndpoint($user);
			$this->request->authorise($newUsername,$newPassword);
			$response = $this->request->get();
			$this->assertNotEqual(200,$response->getStatusCode(),"A removed user resource is still available in the system after deletion.");
		}
		
		function testUserManipulationHandlesUnsupportedRequests(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test unsupported methods ****/
			$response = $this->request->post();
			$this->assertEqual(405,$response->getStatusCode(),"/users/{user_id} endpoint returns incorrect status code for unsupported method POST: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/users/{user_id} endpoint returns incorrect reason for unsupported method POST: {$response->getReason()}");
			$this->validateErrorMessage($response,"$user does not support the POST method.","Making an unsupported POST request to the /users/{user_id} endpoint returns an invalid message");
		}
		
		function testUserManipulationHandlesMissingOrInvalidArguments(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_2"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test Non-JSON Request Body ****/
			$this->request->setBody("Invalid JSON");
			$response = $this->request->put();
			$this->assertEqual(400,$response->getStatusCode(),"/users/{user_id} endpoint returns incorrect status code for invalid JSON request body: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users/{user_id} endpoint returns incorrect reason for invalid JSON request body: {$response->getReason()}");
			$this->validateErrorMessage($response,"Request arguments must be supplied using valid JSON.","PUTing to the /users/{user_id} endpoint with a non-JSON body returns an invalid message");
			
			
			/**** Test Missing Required Arguments ****/
			$this->request->setBody(json_encode(array()));
			$response = $this->request->put();
			$this->assertEqual(400,$response->getStatusCode(),"/users/{user_id} endpoint returns incorrect status code for missing required arguments: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users/{user_id} endpoint returns incorrect reason for missing required arguments: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following arguments are required, but have not been supplied: username, role.","Failing to supply required arguments to the /users/{user_id} endpoint returns an invalid message");
			
			
			/**** Test Invalid Arguments ****/
			$this->request->setBody(json_encode(array("username"=>"no spaces","authentication"=>1234,"role"=>true,"options"=>false)));
			$response = $this->request->put();
			$this->assertEqual(400,$response->getStatusCode(),"/users/{user_id} endpoint returns incorrect status code for invalid arguments: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/users/{user_id} endpoint returns incorrect reason for invalid arguments: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following arguments have invalid values: username, authentication, role, options.","Supplying invalid argument values to the /users/{user_id} endpoint returns an invalid message");
			
			
			/**** Test Duplicate Argument Values ****/
			$this->requestAs("root");
			$this->request->setEndpoint("/apis/14358019264956/users");
			$newUsername = "test_user_3"; $newPassword = "authentication";
			$this->request->setBody(json_encode(array("username"=>$newUsername,"authentication"=>$newPassword,"role"=>"consumer")));
			$this->request->post();
			
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			$newUsername = "test_user_3"; $newPassword = "authentication";
			$this->request->setBody(json_encode(array("username"=>$newUsername,"authentication"=>$newPassword,"role"=>"consumer")));
			
			//Make Duplicate Put
			$response = $this->request->put();
			$this->assertEqual(409,$response->getStatusCode(),"Updating a user resource with a duplicate name returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Conflict",$response->getReason(),"Updating a user resource with a duplicate name returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"A user identified by 'test_user_3' already exists.","Updating a user resource with a duplicate name returns an invalid message");
		}
		
		function testUserManipulationHandlesUnauthorisedRequests(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_4"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test Unauthorised Request ****/
			$this->request->takeFromHeaders("Authorization");
			$response = $this->request->delete();
			$this->assertEqual(401,$response->getStatusCode(),"Unauthorised request to /users/{user_id} endpoint returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Authorization Required",$response->getReason(),"Unauthorised request to /users/{user_id} endpoint returns incorrect reason: {$response->getReason()}");
			$this->assertEqual("Basic",$response->viewFromHeaders("WWW-Authenticate"),"Unauthorised request to /users/{user_id} endpoint does not return correct WWW-Authenticate header: {$response->viewFromHeaders('WWW-Authenticate')}");
			$this->validateErrorMessage($response,"Please use Basic Authentication to authorise this request.","Sending an unauthorised request to the /users/{user_id} endpoint returns an invalid message");
			
			
			/**** Test Incorrectly Authorised Request ****/
			$this->request->setEndpoint($user);
			$this->request->setBody(json_encode(array("username"=>$username,"role"=>"consumer")));
			$this->request->authorise("Fake User","nopass");
			$response = $this->request->put();
			$this->assertEqual(403,$response->getStatusCode(),"Accessing /users/{user_id} endpoint with unauthorised credentials returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Accessing /users/{user_id} endpoint with unauthorised credentials returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The provided tokens do not have permission to perform this action.","Accessing /users/{user_id} endpoint with unauthorised credentials returns an invalid message");
		}
		
		function testUserManipulationNegotiatesContent(){
			/**** Test that correct content types are returned by default ****/
			$this->initialiseRequest();
			
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_5"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user retrieval request did not return the correct content representation or charset by default.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user retrieval request did not return the correct content language by default.");
			
			
			/**** Test that supported content types return the correct representation ****/
			$this->initialiseRequest();
			
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_5"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			$this->request->setEndpoint($user);
			$this->request->addPreferedContentType("application/json");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("en");
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			$this->assertEqual(200,$response->getStatusCode(),"A successful user retrieval request for a supported content type did not return a sucessful status code.");
			$this->assertEqual("OK",$response->getReason(),"A successful user retrieval request for a supported content type did not return a sucessful reason.");
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user retrieval request for a supported content representation and charset were not honoured.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user retrieval request for a supported content language was not honoured.");
			
			
			/**** Test that unsupported content types return an error ****/
			$this->initialiseRequest();
			
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_5"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			$this->request->setEndpoint($user);
			$this->request->addPreferedContentType("text/html");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("fr");
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			$this->assertEqual(406,$response->getStatusCode(),"Requesting unsupported content types from the /users/{user_id} endpoint returns an incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Acceptable",$response->getReason(),"Requesting unsupported content types from the /users/{user_id} endpoint returns an incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested end-point does not support formats: text/html, languages: fr.","Requesting unsupported content types from the /users/{user_id} endpoint returns an incorrect message");
		}
		
		function testManipulationOfMissingUserResources(){
			/**** Test requesting a non-existant resource ****/
			$this->request->setEndpoint("/apis/14358019264956/users/nonexistant");
			$this->request->assureConsistency();
			$this->request->setBody(json_encode(array("username"=>"username","role"=>"consumer")));
			$response = $this->request->put();
			$this->assertEqual(404,$response->getStatusCode(),"Accessing /users/{user_id} endpoint for a non-existant user_id returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Found",$response->getReason(),"Accessing /users/{user_id} endpoint for a non-existant user_id returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"There is presently no resource with the given URI.","Accessing /users/{user_id} endpoint with non-existant user_id returns an invalid message");
		}
		
		function testUserManipulationConcurrencyChecks(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_6"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->request->setEndpoint($user);
			$this->request->authorise($username,$password);
			$validResponse = $this->request->get();
			
			
			/**** Test manipulating a resource without checking concurrency ****/
			$this->request->setBody(json_encode(array("username"=>"another_user","authentication"=>$password,"role"=>"consumer")));
			$response = $this->request->put();
			$this->assertEqual(403,$response->getStatusCode(),"Trying to manipulate /users/{user_id} endpoint without concurrency headers returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Trying to manipulate /users/{user_id} endpoint without concurrency headers returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"If-Unmodified-Since and If-Match Headers must be specified to modify this resource.","Manipulating /users/{user_id} without concurrency headers returns an invalid message");			
			
			
			/**** Test manipulating a resource with invalid concurrency tags ****/
			$newUsername = "another_user"; $newPassword = $password;
			$this->request->assureConsistency($validResponse->viewFromHeaders("Last-Modified"),$validResponse->viewFromHeaders("ETag"));
			$this->request->setBody(json_encode(array("username"=>$newUsername,"authentication"=>$newPassword,"role"=>"consumer")));
			$this->request->put();
			
			//Repeat PUT with stale concurrency headers
			$this->request->authorise($newUsername,$newPassword);
			$response = $this->request->put();
			$this->assertEqual(412,$response->getStatusCode(),"Trying to manipulate /users/{user_id} endpoint with stale concurrency headers returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Precondition Failed",$response->getReason(),"Trying to manipulate /users/{user_id} endpoint with stale concurrency headers returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The wrong Modification Date and ETag values were given for this resource.","Manipulating /users/{user_id} with stale concurrency headers returns an invalid message");
		}
				
		
		function testSuccessfulUserTokensRetrieval(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_10"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			/**** Test User Tokens Resource is correctly retrieved ****/
			$this->request->setEndpoint("$user/tokens");
			$this->request->setBody("");
			$this->request->authorise($username,$password);
			$response = $this->request->get();
			
			//Test for Valid Status Line
			$this->assertEqual(200,$response->getStatusCode(),"Successful user tokens retrieval returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("OK",$response->getReason(),"Successful user tokens retrieval returns the wrong reason: {$response->getReason()}");
			
			//Test for Mandatory Headers
			$datePattern = "/".gmdate("D, d M Y")." [0-2][0-9]:[0-5][0-9]:[0-5][0-9] GMT/";
			$locationPattern = "/http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot)."$user/tokens")."/";
			$this->assertPattern($datePattern,$response->viewFromHeaders("Date"),"Successful user tokens retrieval responds with a malformed or incorrect Date Header: {$response->viewFromHeaders('Date')}");
			$this->assertEqual(strlen($response->getBody()),$response->viewFromHeaders("Content-Length"),"Successful user tokens retrieval responds with the incorrect Content-Length: ".$response->viewFromHeaders("Content-Length"));
			$this->assertPattern($locationPattern,$response->viewFromHeaders("Content-Location"),"Successful user tokens retrieval responds with the incorrect Content-Location: ".$response->viewFromHeaders("Content-Location"));
			
			//Ensure there are no unexpected Headers
			$this->assertEqual(9,sizeof($response->getHeaders(false)),"Successful user tokens retrieval responds with superfluous headers (9 expected, ".sizeof($response->getHeaders())." given)");
			
			//Test Retrieved User Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful user tokens retrieval does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved user's tokens resource does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot))."\/apis\/14358019264956\/users\/[0-9]{14}\/tokens";
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved user's tokens resource includes an invalid self link: {$responseBody['self']}");
				}
					
				$this->assertTrue(array_key_exists("key",$responseBody),"A successfully retrieved user's tokens resource does not include an MD5 key.");
				if(array_key_exists("key",$responseBody)){
					$this->assertPattern("/^[a-f0-9]{32}$/",$responseBody["key"],"A successfully retrieved user's tokens resource has an invalid MD5 key: {$responseBody['key']}");
				}
				
				$this->assertTrue(array_key_exists("secret",$responseBody),"A successfully retrieved user's tokens resource does not include an MD5 secret.");
				if(array_key_exists("secret",$responseBody)){
					$this->assertPattern("/^[a-f0-9]{32}$/",$responseBody["secret"],"A successfully retrieved user's tokens resource does has an invalid MD5 secret: {$responseBody['secret']}");
				}
				
				$this->assertTrue(array_key_exists("expiry",$responseBody),"A successfully retrieved user's tokens resource does not include an expiry timestamp.");
				if(array_key_exists("expiry",$responseBody)){
					$this->assertPattern("/^[0-9]{10}$/",$responseBody["expiry"],"A successfully retrieved user's tokens resource has an invalid expiry timestamp: {$responseBody['expiry']}");
				}
				
				$this->assertTrue(array_key_exists("renew",$responseBody),"A successfully retrieved user's tokens resource does not include a renewal link.");
				if(array_key_exists("renew",$responseBody)){
					$renewalPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."$user/renewals?warrant="))."[a-f0-9]{32}";
					$this->assertPattern("/^$renewalPattern$/",$responseBody["renew"],"A successfully retrieved user's tokens resource has an invalid renewal link: {$responseBody['renew']}");
				}
			}
		}
		
		function testUserTokensRetrievalHandlesUnsupportedRequests(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_11"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$this->request->setBody("");
			$this->request->assureConsistency();
			
			/**** Test unsupported methods ****/
			$this->requestAs("root");
			$response = $this->request->put();
			$this->assertEqual(405,$response->getStatusCode(),"/api/{api_id}/users/{user_id}/tokens endpoint returns incorrect status code for unsupported method PUT: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/api/{api_id}/users/{user_id}/tokens endpoint returns incorrect reason for unsupported method PUT: {$response->getReason()}");
			$this->validateErrorMessage($response,"$user/tokens does not support the PUT method.","Making an unsupported PUT request to the /api/{api_id}/users/{user_id}/tokens endpoint returns an invalid message");
			
			$this->request->setBody(json_encode(array()));
			$response = $this->request->post();
			$this->assertEqual(405,$response->getStatusCode(),"/api/{api_id}/users/{user_id}/tokens endpoint returns incorrect status code for unsupported method POST: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/api/{api_id}/users/{user_id}/tokens endpoint returns incorrect reason for unsupported method POST: {$response->getReason()}");
			$this->validateErrorMessage($response,"$user/tokens does not support the POST method.","Making an unsupported POST request to the /api/{api_id}/users/{user_id}/tokens endpoint returns an invalid message");
			
			$this->request->setBody("");
			$response = $this->request->delete();
			$this->assertEqual(405,$response->getStatusCode(),"/api/{api_id}/users/{user_id}/tokens endpoint returns incorrect status code for unsupported method DELETE: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/api/{api_id}/users/{user_id}/tokens endpoint returns incorrect reason for unsupported method DELETE: {$response->getReason()}");
			$this->validateErrorMessage($response,"$user/tokens does not support the DELETE method.","Making an unsupported DELETE request to the /api/{api_id}/users/{user_id}/tokens endpoint returns an invalid message");
		}
		
		function testUserTokensRetrievalHandlesUnauthorisedRequests(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_13"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$this->request->setBody("");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			
			/**** Test Unauthorised Request ****/
			$this->request->takeFromHeaders("Authorization");
			$response = $this->request->get();
			$this->assertEqual(401,$response->getStatusCode(),"Unauthorised request to /api/{api_id}/users/{user_id}/tokens endpoint returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Authorization Required",$response->getReason(),"Unauthorised request to /api/{api_id}/users/{user_id}/tokens endpoint returns incorrect reason: {$response->getReason()}");
			$this->assertEqual("Basic",$response->viewFromHeaders("WWW-Authenticate"),"Unauthorised request to /api/{api_id}/users/{user_id}/tokens endpoint does not return correct WWW-Authenticate header: {$response->viewFromHeaders('WWW-Authenticate')}");
			$this->validateErrorMessage($response,"Please use Basic Authentication to authorise this request.","Sending an unauthorised request to the /api/{api_id}/users/{user_id}/tokens endpoint returns an invalid message");
			
			
			/**** Test Incorrectly Authorised Request ****/
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise("Fake User","badpass");
			$response = $this->request->put();
			$this->assertEqual(403,$response->getStatusCode(),"Accessing /api/{api_id}/users/{user_id}/tokens endpoint with unauthorised credentials returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Accessing /api/{api_id}/users/{user_id}/tokens endpoint with unauthorised credentials returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The provided tokens do not have permission to perform this action.","Accessing /api/{api_id}/users/{user_id}/tokens endpoint with unauthorised credentials returns an invalid message");
		}
		
		function testUserTokensRetrievalNegotiatesContent(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_14"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			
			/**** Test that correct content types are returned by default ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$this->request->setBody("");
			$response = $this->request->get();
			
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user token retrieval request did not return the correct content representation or charset by default.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user token retrieval request did not return the correct content language by default.");
			
			//Create a new user resource
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_14"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			/**** Test that supported content types return the correct representation ****/
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$this->request->addPreferedContentType("application/json");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("en");
			$response = $this->request->get();
			
			$this->assertEqual(200,$response->getStatusCode(),"A successful user tokens retrieval request for a supported content type did not return a sucessful status code.");
			$this->assertEqual("OK",$response->getReason(),"A successful user tokens retrieval request for a supported content type did not return a sucessful reason.");
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user tokens retrieval request for a supported content representation and charset were not honoured.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user tokens retrieval request for a supported content language was not honoured.");
			
			
			//Create a new user resource
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_14"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			/**** Test that unsupported content types return an error ****/
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$this->request->addPreferedContentType("text/html");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("fr");
			$response = $this->request->get();
			
			$this->assertEqual(406,$response->getStatusCode(),"Requesting unsupported content types from the /api/{api_id}/users/{user_id}/tokens endpoint returns an incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Acceptable",$response->getReason(),"Requesting unsupported content types from the /api/{api_id}/users/{user_id}/tokens endpoint returns an incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested end-point does not support formats: text/html, languages: fr.","Requesting unsupported content types from the /api/{api_id}/users/{user_id}/tokens endpoint returns an incorrect message");
		}

		function testeRetrievalOfMissingUserTokensResources(){
			/**** Test requesting a non-existant resource ****/
			$this->request->setEndpoint("/apis/14358019264956/users/nonexistant/tokens");
			$response = $this->request->get();
			$this->assertEqual(404,$response->getStatusCode(),"Accessing /api/{api_id}/users/{user_id}/tokens endpoint for a non-existant user_id returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Found",$response->getReason(),"Accessing /api/{api_id}/users/{user_id}/tokens endpoint for a non-existant user_id returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"There is presently no resource with the given URI.","Accessing /api/{api_id}/users/{user_id}/tokens endpoint with non-existant user_id returns an invalid message");
		}
		
		function testSuccessfulUserTokensRenewal(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1313"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			//Retrieve initial user token and renewal link
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$this->request->setBody("");
			$response = $this->request->get();
			$responseBody = json_decode($response->getBody(),true);
			$originalKey = $responseBody["key"];
			$originalSecret = $responseBody["secret"];
			$originalExpiry = $responseBody["expiry"];
			$originalRenew = $responseBody["renew"];
			
			//Test token renewal
			$this->requestAs("root");
			$this->request->setEndpoint(str_replace("http://{$this->apiRoot}","",$responseBody["renew"]));
			$this->request->setBody(json_encode(array()));
			$response = $this->request->post();
			
			//Test for Valid Status Line
			$this->assertEqual(201,$response->getStatusCode(),"Successful user tokens renewal returns the wrong status code: {$response->getStatusCode()}");
			$this->assertEqual("Created",$response->getReason(),"Successful user tokens renewal returns the wrong reason: {$response->getReason()}");
			
			//Test for Mandatory Headers
			$datePattern = "/".gmdate("D, d M Y")." [0-2][0-9]:[0-5][0-9]:[0-5][0-9] GMT/";
			$locationPattern = "/http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot)."$user/renewals")."/";
			
			$this->assertPattern($datePattern,$response->viewFromHeaders("Date"),"Successful user tokens renewal responds with a malformed or incorrect Date Header: {$response->viewFromHeaders('Date')}");
			$this->assertEqual(strlen($response->getBody()),$response->viewFromHeaders("Content-Length"),"Successful user tokens renewal responds with the incorrect Content-Length: ".$response->viewFromHeaders("Content-Length"));
			$this->assertPattern($locationPattern,$response->viewFromHeaders("Content-Location"),"Successful user tokens renewal responds with the incorrect Content-Location: ".$response->viewFromHeaders("Content-Location"));
			
			//Ensure there are no unexpected Headers
			$this->assertEqual(8,sizeof($response->getHeaders(false)),"Successful user tokens renewal responds with superfluous headers (8 expected, ".sizeof($response->getHeaders())." given)");
			
			//Test Retrieved User Content
			$this->assertTrue($this->validJSON($response->getBody()),"Successful user tokens renewal does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("key",$responseBody),"A successfully created renewal resource does not include an MD5 key.");
				if(array_key_exists("key",$responseBody)){
					$this->assertPattern("/^[a-f0-9]{32}$/",$responseBody["key"],"A successfully created renewal resource has an invalid MD5 key: {$responseBody['key']}");
					$this->assertNotEqual($responseBody["key"], $originalKey,"A successfully created renewal resource does not re-issue the user token's key: {$responseBody['key']}");
				}
				
				$this->assertTrue(array_key_exists("secret",$responseBody),"A successfully created renewal resource does not include an MD5 secret.");
				if(array_key_exists("secret",$responseBody)){
					$this->assertPattern("/^[a-f0-9]{32}$/",$responseBody["secret"],"A successfully created renewal resource has an invalid MD5 secret: {$responseBody['secret']}");
					$this->assertNotEqual($responseBody["secret"], $originalSecret,"A successfully created renewal resource does not re-issue the user token's secret: {$responseBody['secret']}");
				}
				
				$this->assertTrue(array_key_exists("expiry",$responseBody),"A successfully created renewal resource does not include an expiry timestamp.");
				if(array_key_exists("expiry",$responseBody)){
					$this->assertPattern("/^[0-9]{10}$/",$responseBody["expiry"],"A successfully created renewal resource has an invalid expiry timestamp: {$responseBody['expiry']}");
					$this->assertTrue($responseBody["expiry"] > $originalExpiry,"A successfully created renewal resource does not extend the user token's validity: {$responseBody['expiry']}");
				}
				
				$this->assertTrue(array_key_exists("renew",$responseBody),"A successfully created renewal resource does not include a renewal link.");
				if(array_key_exists("renew",$responseBody)){
					$renewalPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."$user/renewals?warrant=")."[a-f0-9]{32}");
					$this->assertPattern("/$renewalPattern/",$responseBody["renew"],"A successfully created renewal resource has an invalid renewal link: {$responseBody['renew']}");
					$this->assertNotEqual($responseBody["renew"], $originalRenew,"A successfully created renewal resource does not issue a new link for future renewals: {$responseBody['renew']}");
				}
			}
			
			/**** Test renewals collection retrieval ****/
			
			//Renew token multiple times
			$this->request->setEndpoint(str_replace("http://{$this->apiRoot}","",$responseBody["renew"]));
			$this->request->setBody(json_encode(array()));
			$response = $this->request->post();
			$responseBody = json_decode($response->getBody(),true);
			
			$this->request->setEndpoint(str_replace("http://{$this->apiRoot}","",$responseBody["renew"]));
			$response = $this->request->post();
			$responseBody = json_decode($response->getBody(),true);
			
			$this->request->setEndpoint(str_replace("http://{$this->apiRoot}","",$responseBody["renew"]));
			$response = $this->request->post();
			$responseBody = json_decode($response->getBody(),true);
			
			//Get full renewals collection
			$this->request->setEndpoint("$user/renewals");
			$response = $this->request->get();
			
			$this->assertTrue($this->validJSON($response->getBody()),"Retrieving token renewals collection does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved token renewals collection does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."$user/renewals?page=1"));
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved token renewals collection includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("total",$responseBody),"A successfully retrieved token renewals collection does not include the total record count.");
				if(array_key_exists("total",$responseBody)){
					$this->assertEqual(4,$responseBody["total"],"A successfully retrieved token renewals collection includes the wrong total record count: {$responseBody['total']}");
				}
				
				$this->assertTrue(array_key_exists("entries",$responseBody),"A successfully retrieved token renewals collection does not include an array of token renewals.");
				if(array_key_exists("entries",$responseBody)){
					$this->assertEqual(4,sizeof($responseBody["entries"]),"A successfully retrieved token renewals collection includes the incorrect number of renewals: ".sizeof($responseBody['entries']));

				}
			}
			
			//Get paged renewals collection
			$this->request->setEndpoint("$user/renewals");
			$this->request->setBody("");
			$this->request->insertToArguments("page","2");
			$this->request->insertToArguments("records","2");
			$response = $this->request->get();
			
			$this->assertTrue($this->validJSON($response->getBody()),"Retrieving paginated token renewals collection does not return valid JSON in the response body.");
			if($this->validJSON($response->getBody())){
				$responseBody = json_decode($response->getBody(),true);
				
				$this->assertTrue(array_key_exists("self",$responseBody),"A successfully retrieved paginated token renewals collection does not include a self link.");
				if(array_key_exists("self",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."$user/renewals?page=2&records=2"));
					$this->assertPattern("/$userPattern/",$responseBody["self"],"A successfully retrieved paginated token renewals collection includes an invalid self link: {$responseBody['self']}");
				}
				
				$this->assertTrue(array_key_exists("prev",$responseBody),"A successfully retrieved paginated token renewals collection does not include a link to the previous page.");
				if(array_key_exists("prev",$responseBody)){
					$userPattern = "http:\/\/".str_replace("/","\/",preg_quote($this->apiRoot."$user/renewals?page=1&records=2"));
					$this->assertPattern("/$userPattern/",$responseBody["prev"],"A successfully retrieved paginated token renewals collection includes an invalid link to the previous page: {$responseBody['prev']}");
				}
				
				$this->assertTrue(array_key_exists("total",$responseBody),"A successfully retrieved paginated token renewals collection does not include the total record count.");
				if(array_key_exists("total",$responseBody)){
					$this->assertEqual(4,$responseBody["total"],"A successfully retrieved paginated token renewals collection includes the wrong total record count: {$responseBody['total']}");
				}
				
				$this->assertTrue(array_key_exists("entries",$responseBody),"A successfully retrieved paginated token renewals collection does not include an array of renewals.");
				if(array_key_exists("entries",$responseBody)){
					$this->assertEqual(2,sizeof($responseBody["entries"]),"A successfully retrieved paginated token renewals collection includes the incorrect number of users: ".sizeof($responseBody['entries']));
				}
			}
		}
		
		function testUserTokensRenewalHandlesUnsupportedRequests(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1314"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			//Retrieve initial user token and renewal link
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$responseBody = json_decode($this->request->get()->getBody(),true);
			
			//Renew Token
			$this->request->setEndpoint(str_replace("http://{$this->apiRoot}","",$responseBody["renew"]));
			$response = $this->request->post();
			
			/**** Test unsupported methods ****/
			$this->requestAs("root");
			$response = $this->request->put();
			$this->assertEqual(405,$response->getStatusCode(),"/api/{api_id}/users/{user_id}/renewals endpoint returns incorrect status code for unsupported method PUT: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/api/{api_id}/users/{user_id}/renewals endpoint returns incorrect reason for unsupported method PUT: {$response->getReason()}");
			$this->validateErrorMessage($response,"$user/renewals does not support the PUT method.","Making an unsupported PUT request to the /api/{api_id}/users/{user_id}/renewals endpoint returns an invalid message");
			
			$response = $this->request->delete();
			$this->assertEqual(405,$response->getStatusCode(),"/api/{api_id}/users/{user_id}/renewals endpoint returns incorrect status code for unsupported method DELETE: {$response->getStatusCode()}");
			$this->assertEqual("Method Not Allowed",$response->getReason(),"/api/{api_id}/users/{user_id}/renewals endpoint returns incorrect reason for unsupported method DELETE: {$response->getReason()}");
			$this->validateErrorMessage($response,"$user/renewals does not support the DELETE method.","Making an unsupported DELETE request to the /api/{api_id}/users/{user_id}/renewals endpoint returns an invalid message");
		}
		
		function testUserTokensRenewalHandlesMissingOrInvalidQueryParameters(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1300"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			//Retrieve initial user token and renewal link
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$responseBody = json_decode($this->request->get()->getBody(),true);
			$renewal = str_replace("http://{$this->apiRoot}","",$responseBody["renew"]);
			
			$this->request->setEndpoint("$user/renewals");
			
			/**** Test Invalid GET Query Parameters ****/
			$this->requestAs("root");
			$this->request->clearArguments();
			$this->request->insertToArguments("page","page");
			$this->request->insertToArguments("records","records");
			$this->request->setBody("");
			$response = $this->request->get();
			$this->assertEqual(400,$response->getStatusCode(),"/api/{api_id}/users/{user_id}/renewals endpoint returns incorrect status code for invalid query parameters: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/api/{api_id}/users/{user_id}/renewals endpoint returns incorrect reason for invalid query parameters: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following query parameters have invalid values: page, records.","Supplying invalid query parameter values to the /api/{api_id}/users/{user_id}/renewals endpoint returns an invalid message");
			
			/**** Test Missing 'warrant' parameter ****/
			$this->request->setEndpoint("$user/renewals");
			$this->request->setBody(json_encode(array()));
			$response = $this->request->post();
			$this->assertEqual(400,$response->getStatusCode(),"/api/{api_id}/users/{user_id}/renewals endpoint returns incorrect status code for missing query parameters: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/api/{api_id}/users/{user_id}/renewals endpoint returns incorrect reason for missing query parameters: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following query parameters are required, but missing: warrant.","Failing to supply required query parameter values to the /api/{api_id}/users/{user_id}/renewals endpoint returns an invalid message");
		
			
			/**** Test Invalid 'warrant' parameter ****/
			$this->request->setEndpoint("$user/renewals");
			$this->request->insertToArguments("warrant",md5("badwarrant"));
			$response = $this->request->post();
			$this->assertEqual(400,$response->getStatusCode(),"/api/{api_id}/users/{user_id}/renewals endpoint returns incorrect status code for invalid warrant: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/api/{api_id}/users/{user_id}/renewals endpoint returns incorrect reason for invalid warrant: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following query parameters have invalid values: warrant.","Supplying and invalid warrant to the /api/{api_id}/users/{user_id}/renewals endpoint returns an invalid message");
		
			/**** Test using a 'warrant' more than once ****/
			$this->request->setEndpoint($renewal);
			$response = $this->request->post();
			
			//Post again with same warrant
			$response = $this->request->post();
			$this->assertEqual(400,$response->getStatusCode(),"/api/{api_id}/users/{user_id}/renewals endpoint returns incorrect status code for a previously used warrant: {$response->getStatusCode()}");
			$this->assertEqual("Bad Request",$response->getReason(),"/api/{api_id}/users/{user_id}/renewals endpoint returns incorrect reason for a previously used warrant: {$response->getReason()}");
			$this->validateErrorMessage($response,"The following query parameters have invalid values: warrant.","Supplying a previously used warrant to the /api/{api_id}/users/{user_id}/renewals endpoint returns an invalid message");
		}
		
		function testUserTokensRenewalHandlesUnauthorisedRequests(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1315"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			//Retrieve initial user token and renewal link
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			$responseBody = json_decode($this->request->get()->getBody(),true);
			$renewal = str_replace("http://{$this->apiRoot}","",$responseBody["renew"]);
			
			//Renew Token
			$this->request->setEndpoint($renewal);
			
			/**** Test Unauthorised Request ****/
			$this->request->takeFromHeaders("Authorization");
			$this->request->setBody("");
			$response = $this->request->get();
			$this->assertEqual(401,$response->getStatusCode(),"Unauthorised request to /api/{api_id}/users/{user_id}/renewals endpoint returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Authorization Required",$response->getReason(),"Unauthorised request to /api/{api_id}/users/{user_id}/renewals endpoint returns incorrect reason: {$response->getReason()}");
			$this->assertEqual("Basic",$response->viewFromHeaders("WWW-Authenticate"),"Unauthorised request to /api/{api_id}/users/{user_id}/renewals endpoint does not return correct WWW-Authenticate header: {$response->viewFromHeaders('WWW-Authenticate')}");
			$this->validateErrorMessage($response,"Please use Basic Authentication to authorise this request.","Sending an unauthorised request to the /api/{api_id}/users/{user_id}/renewals endpoint returns an invalid message");

			/**** Test Incorrectly Authorised Request ****/
			$this->request->setEndpoint($renewal);
			$this->request->authorise("Fake User","badpass");
			$this->request->setBody(json_encode(array()));
			$response = $this->request->post();
			$this->assertEqual(403,$response->getStatusCode(),"Accessing /api/{api_id}/users/{user_id}/renewals endpoint with unauthorised credentials returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Forbidden",$response->getReason(),"Accessing /api/{api_id}/users/{user_id}/renewals endpoint with unauthorised credentials returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The provided tokens do not have permission to perform this action.","Accessing /api/{api_id}/users/{user_id}/renewals endpoint with unauthorised credentials returns an invalid message");
		}
		
		function testUserTokensRenewalNegotiatesContent(){
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1316"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			//Retrieve initial user token and renewal link
			$this->request->setEndpoint("$user/tokens");
			$this->request->authorise($username,$password);
			
			
			/**** Test that correct content types are returned by default ****/
			$this->request->setEndpoint("$user/renewals");
			$this->request->setBody("");
			$response = $this->request->get();
			
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user token renewal request did not return the correct content representation or charset by default.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user token renewal request did not return the correct content language by default.");
			
			
			/**** Test that supported content types return the correct representation ****/
			$this->initialiseRequest();
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1316"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			$this->request->setEndpoint("$user/renewals");
			$this->request->addPreferedContentType("application/json");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("en");
			$this->request->setBody("");
			$response = $this->request->get();
			
			$this->assertEqual(200,$response->getStatusCode(),"A successful user tokens renewal request for a supported content type did not return a sucessful status code.");
			$this->assertEqual("OK",$response->getReason(),"A successful user tokens renewal request for a supported content type did not return a sucessful reason.");
			$this->assertEqual("application/json;charset=utf-8",$response->viewFromHeaders("Content-Type"),"A successful user tokens renewal request for a supported content representation and charset were not honoured.");
			$this->assertEqual("en",$response->viewFromHeaders("Content-Language"),"A successful user tokens renewal request for a supported content language was not honoured.");
			
			/**** Test that unsupported content types return an error ****/
			$this->initialiseRequest();
			//Create a new user resource
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1316"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			$this->request->setEndpoint("$user/renewals");
			$this->request->addPreferedContentType("text/html");
			$this->request->addPreferedCharset("UTF-8");
			$this->request->addPreferedLanguage("fr");
			$this->request->setBody("");
			$response = $this->request->get();
			
			$this->assertEqual(406,$response->getStatusCode(),"Requesting unsupported content types from the /api/{api_id}/users/{user_id}/renewals endpoint returns an incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Acceptable",$response->getReason(),"Requesting unsupported content types from the /api/{api_id}/users/{user_id}/renewals endpoint returns an incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"The requested end-point does not support formats: text/html, languages: fr.","Requesting unsupported content types from the /api/{api_id}/users/{user_id}/renewals endpoint returns an incorrect message");
		}
		
		function testRenewalOfMissingUserTokenResources(){
			/**** Test requesting a non-existant resource ****/
			$this->request->setEndpoint("/apis/14358019264956/users/nonexistant/renewals");
			$this->request->assureConsistency("","");
			$response = $this->request->get();
			$this->assertEqual(404,$response->getStatusCode(),"Accessing /api/{api_id}/users/{user_id}/renewals endpoint for a non-existant user_id returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Found",$response->getReason(),"Accessing /api/{api_id}/users/{user_id}/renewals endpoint for a non-existant user_id returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"There is presently no resource with the given URI.","Accessing /api/{api_id}/users/{user_id}/renewals endpoint with non-existant user_id returns an invalid message");
		
			
			/**** Test requesting a deleted (gone) resource ****/
			$this->initialiseRequest();
			$this->request->setEndpoint("/apis/14358019264956/users");
			$username = "test_user_1318"; $password = "authentication";
			$this->request->setBody(json_encode(array("username"=>$username,"authentication"=>$password,"role"=>"consumer")));
			$user = str_replace("http://{$this->apiRoot}","",$this->request->post()->viewFromHeaders("Location"));
			
			$this->request->setEndpoint($user);
			$this->request->setBody("");
			$response = $this->request->get();
			$this->request->assureConsistency($response->viewFromHeaders("Last-Modified"),$response->viewFromHeaders("ETag"));
			
			$this->request->delete();
			$response = $this->request->get();;
			$this->assertEqual(404,$response->getStatusCode(),"Accessing /api/{api_id}/users/{user_id}/renewals endpoint after the user has been removed returns incorrect status code: {$response->getStatusCode()}");
			$this->assertEqual("Not Found",$response->getReason(),"Accessing /api/{api_id}/users/{user_id}/renewals endpoint after the user has been removed returns incorrect reason: {$response->getReason()}");
			$this->validateErrorMessage($response,"There is presently no resource with the given URI.","Accessing /api/{api_id}/users/{user_id}/renewals endpoint after the user has been removed returns an invalid message");			
		}
	}
?>