<?php
	class Sorter extends WadadliClass{
		/** @Collection(type=@Object(class='Criterion')) */
		protected $criteria;
		
		/*
		 * Adds a Criterion object to the criteria collection, converting non-string values
		 * to strings where applicable.
		 */
		public function addCriterion($field, $order){
			if(!is_string($field)){
				warning(UNEXPECTED_ARGUMENT_WARNING, "Sorter criterion should have string fields, ".gettype($field)." given.");
				return;
			}
			
			if(!is_string($order)){
				warning(UNEXPECTED_ARGUMENT_WARNING, "Sorter criterion should have string order, ".gettype($order)." given.");
				return;
			}
			
			$this->appendToCriteria(new Criterion($field, $order, array()));
		}
		
		/*
		 * Removes all criteria from the criteria collection
		 */
		public function clearCriteria(){
			$this->setCriteria(array());
		}
	}
?>