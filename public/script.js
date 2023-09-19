// Declare variables for the game
let word;                // The word to guess
let guessesLeft;         // Number of guesses left
let incorrectGuesses;   // Array to store incorrect guesses
let correctGuesses;      // Array to store correct guesses

// Start the wordle game
function startGame() {
  // Fetch a random word from the '/randomword' endpoint
  fetch("/randomword")
    .then(response => response.text())
    .then(data => {
      word = data.toUpperCase().trim();  // Store the random word
      alert(word);  // Display the word (for debugging)
      guessesLeft = 7;  // Set the initial number of guesses
      incorrectGuesses = [];  // Initialize incorrect guesses array
      correctGuesses = Array.from({ length: word.length }, () => "_");  // Initialize correct guesses array with underscores

      // Display the initial word with underscores
      const wordElement = document.querySelector("#word");
      wordElement.textContent = correctGuesses.join(" ");

      // Render the hangman (initially empty)
      renderHangman();

      // Set initial guesses and game message
      setGuessesAndMessage();

      // Reset the guess input field
      resetGuessInput();

      // Focus on the guess input field
      focusGuessInput();
    })
    .catch(error => console.error("Error:", error));  // Handle errors
}

// JavaScript functions to show and hide the game over popup
function showGameOverPopup() {
  var popup = document.getElementById("gameOverPopup");
  popup.style.display = "block";  // Show the game over popup
}

function hideGameOverPopup() {
  var popup = document.getElementById("gameOverPopup");
  popup.style.display = "none";  // Hide the game over popup
}

// Function to handle a player's guess
function makeGuess() {
  const guessInput = document.querySelector("#guessInput");
  const guess = guessInput.value.toUpperCase();  // Get the guess and convert to uppercase

  // Check if the guess is valid or has already been guessed
  if (!isValidGuess(guess) || isAlreadyGuessed(guess)) {
    alert("Invalid input!");
    guessInput.value = "";
    return;
  }

  if (!word.includes(guess)) {
    // If the guess is incorrect, add it to the incorrect guesses array
    incorrectGuesses.push(guess);
    guessesLeft--;  // Decrease the number of guesses left
    renderHangman();  // Update the hangman display
  } else {
    // If the guess is correct, update the correct guesses array
    for (let i = 0; i < word.length; i++) {
      if (word[i] === guess) correctGuesses[i] = guess;
    }
    const wordElement = document.querySelector("#word");
    wordElement.textContent = correctGuesses.join(" ");  // Update the displayed word
  }

  setGuessesAndMessage();  // Update guesses and message

  if (guessesLeft === 0)
    endGame("Game Over! You lost. The word was: " + word);  // End the game if no guesses left
  else if (!correctGuesses.includes("_")) {
    endGame("Congratulations! You won! Have a cat to play with!");  // End the game if the word is guessed
    renderCat();  // Display a cat when the player wins
  }

  resetGuessInput();  // Reset the guess input field
  focusGuessInput();  // Focus on the guess input field
}

// Function to handle key presses (Enter key for making a guess)
function handleKeyPress(event) {
  if (event.keyCode === 13) {
    makeGuess();  // Call makeGuess() when Enter key is pressed
  }
}

// Function to check if a guess is valid (single uppercase letter)
function isValidGuess(guess) {
  const pattern = /^[A-Z]$/;
  return pattern.test(guess);
}

// Function to check if a guess has already been guessed (correct or incorrect)
function isAlreadyGuessed(guess) {
  return (
    incorrectGuesses.includes(guess) || correctGuesses.includes(guess)
  );
}

// Function to render the hangman display
function renderHangman() {
  // Hangman steps (ASCII art)
  const catSteps = [
    `
  `,
    `
  /\\_/\\
    `,
    `
  /\\_/\\
 (     )
    `,
    `
  /\\_/\\
 ( o.  )
    `,
    `
  /\\_/\\
 ( o.o )
    `,
    `
  /\\_/\\
  ( o.o )
  > ^ <
 YOU LOSE
`
  ];

  // Get the hangman step based on the remaining guesses
  const hangman = catSteps[7 - guessesLeft];
  const hangmanElement = document.querySelector("#hangman");
  hangmanElement.innerHTML = `<pre>${hangman}</pre>`;

  if (guessesLeft === 2) {
    // If the game is lost, remove the input element and show the game over popup
    const inputElement = document.querySelector("#input");
    inputElement.remove();
    showGameOverPopup();
  }
}

// Function to render a cat GIF
function renderCat() {
  const catGif =
    '<model-viewer src="/Puss_in_Boots.glb" ar shadow-intensity="1" camera-controls touch-action="pan-y"></model-viewer>';
  const hangmanElement = document.querySelector("#hangman");
  hangmanElement.innerHTML = catGif;
}

// Function to set the displayed guesses and message
function setGuessesAndMessage() {
  const incorrectGuessesElement = document.querySelector("#guesses");
  incorrectGuessesElement.textContent = `Incorrect Guesses: ${incorrectGuesses.join(
    ", "
  )}`;
}

// Function to reset the guess input field
function resetGuessInput() {
  const guessInputElement = document.querySelector("#guessInput");
  guessInputElement.value = "";
}

// Function to focus on the guess input field
function focusGuessInput() {
  const guessInputElement = document.querySelector("#guessInput");
  guessInputElement.focus();
}

// Function to end the game with a message
function endGame(message) {
  const guessInputElement = document.querySelector("#guessInput");
  guessInputElement.disabled = true;  // Disable the guess input field

  const messageElement = document.querySelector("#message");
  messageElement.textContent = message;  // Set the game message

  if (message.includes("won")) {
    renderCat();  // Display a cat when the player wins
  }
}

// Start the game when the page loads
startGame();
