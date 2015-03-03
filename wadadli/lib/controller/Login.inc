<?php
	class Login extends Controller{
		public function execute(){
			$redirect = "http://{$_SERVER['HTTP_HOST']}{$_SERVER['REQUEST_URI']}";
			
			//If this session already has an active user, redirect to homepage
			if(SessionManager::activeUser()){
				header("Location:http://".SettingsManager::getSetting('install','urlRoot'));
				return;
			}
			
			//If a username and password was submitted, attempt to login
			if(array_key_exists('username',$_POST) && array_key_exists('password',$_POST)){
				$username = $_POST['username'];
				$password = md5($_POST['password']);
				
				if(array_key_exists('redirect',$_POST) && filter_var($_POST['redirect'],FILTER_VALIDATE_URL)){
					$redirect = $_POST['redirect'];
				}
				
				$searcher = new Searcher();
				$searcher->addCriterion("username",Criterion::EQUAL,$username);
			
				$sqlGateway = new SQLGateway();
				$user = $sqlGateway->findUnique('User',$searcher);
				
				if(!$user){
					warning("The username/password combination entered is incorrect.","");
				}else if($user->getDisabled()){
					warning("This user account has been disabled. Please contact a system administrator to regain access.","");
				}else if($user->getPassword() != $password){
					warning("The username/password combination entered is incorrect.","");
				}else{
					SessionManager::changeUser($user);
					header("Location:$redirect");
					return;
				}
			}
			
			$loginPage = new PageTemplate();
			$loginPage->setView(new LoginView($redirect));
			$loginPage->render();
		}
	}
?>