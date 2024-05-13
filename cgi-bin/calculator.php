#!/usr/bin/php
<?php

class Field_calculate {
    const PATTERN = '/(?:\-?\d+(?:\.?\d+)?[\+\-\*\/])+\-?\d+(?:\.?\d+)?/';

    const PARENTHESIS_DEPTH = 10;

    public function calculate($input){
        if(strpos($input, '+') != null || strpos($input, '-') != null || strpos($input, '/') != null || strpos($input, '*') != null){
            //  Remove white spaces and invalid math chars
            $input = str_replace(',', '.', $input);
            $input = preg_replace('[^0-9\.\+\-\*\/\(\)]', '', $input);

            //  Calculate each of the parenthesis from the top
            $i = 0;
            while(strpos($input, '(') || strpos($input, ')')){
                $input = preg_replace_callback('/\(([^\(\)]+)\)/', 'self::callback', $input);

                $i++;
                if($i > self::PARENTHESIS_DEPTH){
                    break;
                }
            }

            //  Calculate the result
            if(preg_match(self::PATTERN, $input, $match)){
                return $this->compute($match[0]);
            }
            // To handle the special case of expressions surrounded by global parenthesis like "(1+1)"
            if(is_numeric($input)){
                return $input;
            }

            return 0;
        }

        return $input;
    }

	private function compute($input) {
		// Define an anonymous function to evaluate the input expression
		$evaluator = function() use ($input) {
			return eval('return ' . $input . ';');
		};
	
		// Call the anonymous function to get the result
		return $evaluator();
	}

    private function callback($input){
        if(is_numeric($input[1])){
            return $input[1];
        }
        elseif(preg_match(self::PATTERN, $input[1], $match)){
            return $this->compute($match[0]);
        }

        return 0;
    }
}


function replaceSpaces($expression) {
    return str_replace(' ', '+', $expression);
}

// Check if the QUERY_STRING environment variable is set
if (isset($_SERVER['QUERY_STRING'])) {
    // Parse the query string to extract parameters
    parse_str($_SERVER['QUERY_STRING'], $queryStringParams);

    // Check if the 'expression' parameter is present
    if (isset($queryStringParams['expression'])) {
        $Cal = new Field_calculate();

        $expression = $queryStringParams['expression'];
		$expression = str_replace(' ', '+', $expression);
        $result = $Cal->calculate($expression);
        echo $result;
    } else {
        // Handle the case where the 'expression' parameter is missing
        echo "Error: 'expression' parameter is missing";
    }
} else {
    // Handle the case where the QUERY_STRING is not set
    echo "Error: QUERY_STRING environment variable is not set";
}

?>
