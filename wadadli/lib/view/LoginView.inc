<?php
	class LoginView extends ColumnView{
		public function __construct($redirect){
			parent::__construct("loginView",1,false,false);
			
			$loginForm = new WadadliForm("loginForm","","Login","Login");
			$loginForm->addInput(new WadadliHiddenField("Redirect",$redirect));
			$loginForm->addInput(new WadadliTextField("Username"));
			$loginForm->addInput(new WadadliPasswordField("Password","",true));
			$this->addToColumn(1,$loginForm);
		}
	}
?>