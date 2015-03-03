<?php
	class WadadliRatingStars extends WadadliInputComponent{
		/** @Integer(default=5) */
		protected $stars;
		
		/** @Boolean(default=false) */
		protected $readOnly;
		
		private $postName;
		
		public function changePostName($newPostName){
			$this->postName = (string)$newPostName;
		}
	
		protected function getEntryComponent(){
			$validLabel = ($this->postName || $this->postName == "0");
			$componentTitle = $validLabel?$this->postName:decapitalise(str_replace(" ","",$this->getTitle()));
			
			$entryComponent = new WadadliContainer("{$this->getComponentId()}RatingStars");
			
			for($i=1; $i <= $this->getStars(); $i++){
				if($i <= $this->getDefaultValue()){
					$starType = "img/rated_star.png";
				}else if($i-1 < $this->getDefaultValue()){
					$starType = "img/half_rated_star.png";
				}else{
					$starType = "img/unrated_star.png";
				}
				$star = new WadadliImageComponent("",$starType);
				if(!$this->isReadOnly()){
					$star->onClick('updateStarRating',array($i));
				}
				$entryComponent->addContent($star);
			}
			
			$hiddenField = new WadadliContainer();
			$hiddenField->addClass('ratingValue');
			$hiddenField->setTag('input');
			$hiddenField->addAttribute('type','hidden');
			$hiddenField->addAttribute('name',$componentTitle);
			$hiddenField->addAttribute('value',$this->getDefaultValue());
			$entryComponent->addContent($hiddenField);
			
			return $entryComponent;
		}
	}
?>