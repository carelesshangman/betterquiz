// Quiz state
let quizData = null;
let currentQuestionIndex = 0;
let userAnswers = [];
let score = 0;
let celebrationCounter = 0; // Track which celebration to show
let animationsEnabled = true; // Track animation state
let questionMappings = []; // Store original answer indices after shuffling
let catBreakInterval = null; // Store interval for cat rotation
let catCountdownInterval = null; // Store countdown interval

// Sample quiz data - Load from external JSON file
let sampleQuiz = null;

// Load the default quiz from the JSON file
fetch('spo_quiz_json (1).json')
    .then(response => response.json())
    .then(data => {
        sampleQuiz = data;
    })
    .catch(error => {
        console.error('Error loading default quiz:', error);
        // Fallback to a minimal quiz if file can't be loaded
        sampleQuiz = {
            "quiz": {
                "title": "SPO Kviz - Sistemi in Programska Oprema",
                "description": "105 teoretičnih vprašanj + 11 vprašanj o bitnih operacijah",
                "questions": [
                    {
                        "id": 1,
                        "question": "Kaj je datotečni opisnik?",
                        "options": [
                            "Nenegativno celo število, ki predstavlja odprto datoteko s stališča jedra",
                            "Kazalec na datoteko v uporabniškem prostoru",
                            "Struktura, ki vsebuje ime datoteke",
                            "Podatkovni tip za shranjevanje vsebine datoteke"
                        ],
                        "correctAnswer": 0
                    }
                ]
            }
        };
    });

// DOM Elements
const uploadSection = document.getElementById('upload-section');
const quizSection = document.getElementById('quiz-section');
const resultsSection = document.getElementById('results-section');
const uploadZone = document.getElementById('upload-zone');
const fileInput = document.getElementById('file-input');
const loadSampleBtn = document.getElementById('load-sample');
const quizTitle = document.getElementById('quiz-title');
const questionText = document.getElementById('question-text');
const optionsContainer = document.getElementById('options-container');
const currentQuestionSpan = document.getElementById('current-question');
const totalQuestionsSpan = document.getElementById('total-questions');
const progressFill = document.getElementById('progress-fill');
const prevButton = document.getElementById('prev-button');
const nextButton = document.getElementById('next-button');
const submitButton = document.getElementById('submit-button');
const reviewButton = document.getElementById('review-button');
const restartButton = document.getElementById('restart-button');
const toggleAnimationsBtn = document.getElementById('toggle-animations');
const catBreakBtn = document.getElementById('cat-break-btn');
const catModal = document.getElementById('cat-modal');
const closeCatModalBtn = document.getElementById('close-cat-modal');
const catImage = document.getElementById('cat-image');
const catBreed = document.getElementById('cat-breed');
const catCountdown = document.getElementById('cat-countdown');
const catApiKeyInput = document.getElementById('cat-api-key');

// Event Listeners
uploadZone.addEventListener('click', () => fileInput.click());
fileInput.addEventListener('change', handleFileSelect);
loadSampleBtn.addEventListener('click', () => loadQuiz(sampleQuiz));
prevButton.addEventListener('click', () => navigateQuestion(-1));
nextButton.addEventListener('click', () => navigateQuestion(1));
submitButton.addEventListener('click', showResults);
reviewButton.addEventListener('click', reviewAnswers);
restartButton.addEventListener('click', restart);
toggleAnimationsBtn.addEventListener('click', toggleAnimations);
catBreakBtn.addEventListener('click', openCatBreak);
closeCatModalBtn.addEventListener('click', closeCatBreak);

// Close cat modal when clicking outside
catModal.addEventListener('click', (e) => {
    if (e.target === catModal) {
        closeCatBreak();
    }
});

// Drag and drop
uploadZone.addEventListener('dragover', (e) => {
    e.preventDefault();
    uploadZone.classList.add('drag-over');
});

uploadZone.addEventListener('dragleave', () => {
    uploadZone.classList.remove('drag-over');
});

uploadZone.addEventListener('drop', (e) => {
    e.preventDefault();
    uploadZone.classList.remove('drag-over');
    const files = e.dataTransfer.files;
    if (files.length > 0) {
        handleFile(files[0]);
    }
});

// File handling
function handleFileSelect(e) {
    const file = e.target.files[0];
    if (file) {
        handleFile(file);
    }
}

function handleFile(file) {
    if (!file.name.endsWith('.json')) {
        alert('Please upload a JSON file!');
        return;
    }

    const reader = new FileReader();
    reader.onload = (e) => {
        try {
            const data = JSON.parse(e.target.result);
            loadQuiz(data);
        } catch (error) {
            alert('Invalid JSON file! Please check the format.');
            console.error(error);
        }
    };
    reader.readAsText(file);
}

// Toggle animations
function toggleAnimations() {
    animationsEnabled = !animationsEnabled;

    if (animationsEnabled) {
        document.body.classList.remove('no-animations');
        toggleAnimationsBtn.classList.remove('disabled');
        toggleAnimationsBtn.querySelector('.label').textContent = 'Animations: ON';
    } else {
        document.body.classList.add('no-animations');
        toggleAnimationsBtn.classList.add('disabled');
        toggleAnimationsBtn.querySelector('.label').textContent = 'Animations: OFF';
    }
}

// Shuffle array using Fisher-Yates algorithm
function shuffleArray(array) {
    const shuffled = [...array];
    for (let i = shuffled.length - 1; i > 0; i--) {
        const j = Math.floor(Math.random() * (i + 1));
        [shuffled[i], shuffled[j]] = [shuffled[j], shuffled[i]];
    }
    return shuffled;
}

// Load quiz
function loadQuiz(data) {
    if (!data.quiz || !data.quiz.questions || data.quiz.questions.length === 0) {
        alert('Invalid quiz format! Make sure your JSON has a "quiz" object with "questions" array.');
        return;
    }

    quizData = data.quiz;
    currentQuestionIndex = 0;
    userAnswers = new Array(quizData.questions.length).fill(null);
    questionMappings = [];
    score = 0;

    // Randomize answers for each question
    quizData.questions.forEach((question, qIndex) => {
        const indices = question.options.map((_, i) => i);
        const shuffledIndices = shuffleArray(indices);

        // Store the mapping
        questionMappings[qIndex] = {
            shuffledToOriginal: shuffledIndices,
            originalToShuffled: {}
        };

        // Create reverse mapping
        shuffledIndices.forEach((originalIndex, shuffledIndex) => {
            questionMappings[qIndex].originalToShuffled[originalIndex] = shuffledIndex;
        });

        // Shuffle the options
        const originalOptions = [...question.options];
        question.shuffledOptions = shuffledIndices.map(i => originalOptions[i]);
    });

    // Show quiz section
    uploadSection.classList.remove('active');
    quizSection.classList.add('active');

    // Initialize quiz
    quizTitle.textContent = quizData.title || 'Quiz';
    totalQuestionsSpan.textContent = quizData.questions.length;

    displayQuestion();
}

// Display question
function displayQuestion() {
    const question = quizData.questions[currentQuestionIndex];

    // Update progress
    currentQuestionSpan.textContent = currentQuestionIndex + 1;
    const progress = ((currentQuestionIndex + 1) / quizData.questions.length) * 100;
    progressFill.style.width = `${progress}%`;

    // Update question
    questionText.textContent = question.question;

    // Clear and create options (use shuffled options)
    optionsContainer.innerHTML = '';
    const optionsToDisplay = question.shuffledOptions || question.options;

    optionsToDisplay.forEach((option, index) => {
        const optionDiv = document.createElement('div');
        optionDiv.className = 'option';
        optionDiv.textContent = option;
        optionDiv.setAttribute('data-index', index);

        // Highlight if already selected
        if (userAnswers[currentQuestionIndex] === index) {
            optionDiv.classList.add('selected');
        }

        optionDiv.addEventListener('click', () => selectOption(index));
        optionsContainer.appendChild(optionDiv);
    });

    // Update navigation buttons
    prevButton.disabled = currentQuestionIndex === 0;

    if (currentQuestionIndex === quizData.questions.length - 1) {
        nextButton.style.display = 'none';
        submitButton.style.display = 'inline-block';
    } else {
        nextButton.style.display = 'inline-block';
        submitButton.style.display = 'none';
    }

    // Enable next/submit if question is answered
    const isAnswered = userAnswers[currentQuestionIndex] !== null;
    nextButton.disabled = !isAnswered;
    submitButton.disabled = userAnswers.some(answer => answer === null);
}

// Select option
function selectOption(index) {
    const question = quizData.questions[currentQuestionIndex];
    const mapping = questionMappings[currentQuestionIndex];

    // Convert shuffled index to original index
    const originalAnswerIndex = mapping.shuffledToOriginal[index];
    const isCorrect = originalAnswerIndex === question.correctAnswer;

    userAnswers[currentQuestionIndex] = index;

    // Update UI
    const options = optionsContainer.querySelectorAll('.option');
    options.forEach((opt, i) => {
        opt.classList.remove('selected', 'correct', 'incorrect');
        opt.style.pointerEvents = 'none'; // Disable further clicks
    });

    const selectedOption = options[index];

    if (isCorrect) {
        selectedOption.classList.add('correct');
        // Trigger celebration animation only if enabled
        if (animationsEnabled) {
            triggerCelebration();
        }
    } else {
        selectedOption.classList.add('incorrect');
        // Show correct answer after a brief delay
        const correctShuffledIndex = mapping.originalToShuffled[question.correctAnswer];
        setTimeout(() => {
            options[correctShuffledIndex].classList.add('correct');
        }, 500);
    }

    // Re-enable clicks and enable next button after animation
    setTimeout(() => {
        options.forEach(opt => {
            opt.style.pointerEvents = 'auto';
        });
        nextButton.disabled = false;
        submitButton.disabled = userAnswers.some(answer => answer === null);
    }, 1000);
}

// Celebration animations
function triggerCelebration() {
    const celebrations = [
        createConfetti,
        createFireworks,
        createStars,
        createHearts,
        createSparkles
    ];

    // Cycle through different celebrations
    celebrations[celebrationCounter % celebrations.length]();
    celebrationCounter++;
}

function createConfetti() {
    const colors = ['#b794f6', '#f687b3', '#63b3ed', '#68d391', '#f6ad55'];
    const confettiCount = 50;

    for (let i = 0; i < confettiCount; i++) {
        const confetti = document.createElement('div');
        confetti.className = 'confetti';
        confetti.style.left = Math.random() * 100 + 'vw';
        confetti.style.backgroundColor = colors[Math.floor(Math.random() * colors.length)];
        confetti.style.animationDelay = Math.random() * 0.5 + 's';
        confetti.style.animationDuration = (Math.random() * 2 + 2) + 's';
        document.body.appendChild(confetti);

        setTimeout(() => confetti.remove(), 4000);
    }
}

function createFireworks() {
    const colors = ['#b794f6', '#f687b3', '#63b3ed', '#68d391', '#f6ad55'];

    for (let i = 0; i < 3; i++) {
        setTimeout(() => {
            const firework = document.createElement('div');
            firework.className = 'firework';
            firework.style.left = (20 + Math.random() * 60) + '%';
            firework.style.top = (20 + Math.random() * 40) + '%';

            for (let j = 0; j < 12; j++) {
                const particle = document.createElement('div');
                particle.className = 'firework-particle';
                particle.style.backgroundColor = colors[Math.floor(Math.random() * colors.length)];
                particle.style.transform = `rotate(${j * 30}deg)`;
                firework.appendChild(particle);
            }

            document.body.appendChild(firework);
            setTimeout(() => firework.remove(), 2000);
        }, i * 300);
    }
}

function createStars() {
    const starCount = 30;

    for (let i = 0; i < starCount; i++) {
        const star = document.createElement('div');
        star.className = 'star';
        star.textContent = '⭐';
        star.style.left = Math.random() * 100 + 'vw';
        star.style.top = Math.random() * 100 + 'vh';
        star.style.animationDelay = Math.random() * 0.5 + 's';
        document.body.appendChild(star);

        setTimeout(() => star.remove(), 2000);
    }
}

function createHearts() {
    const heartCount = 25;

    for (let i = 0; i < heartCount; i++) {
        const heart = document.createElement('div');
        heart.className = 'heart';
        heart.textContent = '💖';
        heart.style.left = Math.random() * 100 + 'vw';
        heart.style.animationDelay = Math.random() * 0.8 + 's';
        document.body.appendChild(heart);

        setTimeout(() => heart.remove(), 3000);
    }
}

function createSparkles() {
    const sparkleCount = 40;
    const colors = ['#b794f6', '#f687b3', '#63b3ed', '#68d391', '#ffd700'];

    for (let i = 0; i < sparkleCount; i++) {
        const sparkle = document.createElement('div');
        sparkle.className = 'sparkle';
        sparkle.style.left = Math.random() * 100 + 'vw';
        sparkle.style.top = Math.random() * 100 + 'vh';
        sparkle.style.backgroundColor = colors[Math.floor(Math.random() * colors.length)];
        sparkle.style.animationDelay = Math.random() * 0.3 + 's';
        document.body.appendChild(sparkle);

        setTimeout(() => sparkle.remove(), 1500);
    }
}

// Navigate questions
function navigateQuestion(direction) {
    currentQuestionIndex += direction;
    if (currentQuestionIndex < 0) currentQuestionIndex = 0;
    if (currentQuestionIndex >= quizData.questions.length) {
        currentQuestionIndex = quizData.questions.length - 1;
    }
    displayQuestion();
}

// Show results
function showResults() {
    // Calculate score
    score = 0;
    quizData.questions.forEach((question, index) => {
        const mapping = questionMappings[index];
        const originalAnswerIndex = mapping.shuffledToOriginal[userAnswers[index]];
        if (originalAnswerIndex === question.correctAnswer) {
            score++;
        }
    });

    const percentage = Math.round((score / quizData.questions.length) * 100);

    // Show results section
    quizSection.classList.remove('active');
    resultsSection.classList.add('active');

    // Update score
    document.getElementById('score-percentage').textContent = `${percentage}%`;
    document.getElementById('score-fraction').textContent = `${score} / ${quizData.questions.length}`;

    // Animate score circle
    const circumference = 2 * Math.PI * 90;
    const offset = circumference - (percentage / 100) * circumference;

    // Create SVG gradient if not exists
    if (!document.querySelector('#scoreGradient')) {
        const svg = document.querySelector('.score-circle svg');
        const defs = document.createElementNS('http://www.w3.org/2000/svg', 'defs');
        const gradient = document.createElementNS('http://www.w3.org/2000/svg', 'linearGradient');
        gradient.setAttribute('id', 'scoreGradient');

        const stop1 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
        stop1.setAttribute('offset', '0%');
        stop1.setAttribute('style', 'stop-color:#68d391;stop-opacity:1');

        const stop2 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
        stop2.setAttribute('offset', '100%');
        stop2.setAttribute('style', 'stop-color:#63b3ed;stop-opacity:1');

        gradient.appendChild(stop1);
        gradient.appendChild(stop2);
        defs.appendChild(gradient);
        svg.insertBefore(defs, svg.firstChild);
    }

    document.documentElement.style.setProperty('--score-offset', offset);

    // Display detailed results
    displayDetailedResults();
}

// Display detailed results
function displayDetailedResults() {
    const resultsDetails = document.getElementById('results-details');
    resultsDetails.innerHTML = '<h3 style="margin-bottom: 20px; color: var(--accent-blue);">📊 Detailed Results</h3>';

    quizData.questions.forEach((question, index) => {
        const mapping = questionMappings[index];
        const shuffledOptions = question.shuffledOptions || question.options;
        const userShuffledAnswer = userAnswers[index];
        const originalAnswerIndex = mapping.shuffledToOriginal[userShuffledAnswer];
        const isCorrect = originalAnswerIndex === question.correctAnswer;

        const resultItem = document.createElement('div');
        resultItem.className = `result-item ${isCorrect ? 'correct' : 'incorrect'}`;

        const icon = isCorrect ? '✅' : '❌';
        const resultHTML = `
            <div class="result-question">${icon} Question ${index + 1}: ${question.question}</div>
            <div class="result-answer">
                <strong>Your answer:</strong> ${shuffledOptions[userShuffledAnswer] || 'Not answered'}
            </div>
            ${!isCorrect ? `
                <div class="result-answer" style="color: var(--accent-green);">
                    <strong>Correct answer:</strong> ${question.options[question.correctAnswer]}
                </div>
            ` : ''}
        `;

        resultItem.innerHTML = resultHTML;
        resultsDetails.appendChild(resultItem);
    });
}

// Review answers
function reviewAnswers() {
    resultsSection.classList.remove('active');
    quizSection.classList.add('active');
    currentQuestionIndex = 0;
    displayReviewQuestion();
}

// Display review question
function displayReviewQuestion() {
    const question = quizData.questions[currentQuestionIndex];
    const mapping = questionMappings[currentQuestionIndex];
    const shuffledOptions = question.shuffledOptions || question.options;

    // Update progress
    currentQuestionSpan.textContent = currentQuestionIndex + 1;
    const progress = ((currentQuestionIndex + 1) / quizData.questions.length) * 100;
    progressFill.style.width = `${progress}%`;

    // Update question
    questionText.textContent = question.question;

    // Clear and create options with correct/incorrect indicators
    optionsContainer.innerHTML = '';
    shuffledOptions.forEach((option, shuffledIndex) => {
        const optionDiv = document.createElement('div');
        optionDiv.className = 'option disabled';
        optionDiv.textContent = option;

        const userShuffledAnswer = userAnswers[currentQuestionIndex];
        const originalIndex = mapping.shuffledToOriginal[shuffledIndex];
        const correctShuffledIndex = mapping.originalToShuffled[question.correctAnswer];

        if (shuffledIndex === correctShuffledIndex) {
            optionDiv.classList.add('correct');
            optionDiv.textContent = `✅ ${option}`;
        }

        if (shuffledIndex === userShuffledAnswer && userShuffledAnswer !== correctShuffledIndex) {
            optionDiv.classList.add('incorrect');
            optionDiv.textContent = `❌ ${option}`;
        }

        optionsContainer.appendChild(optionDiv);
    });

    // Update navigation
    prevButton.disabled = currentQuestionIndex === 0;
    nextButton.disabled = currentQuestionIndex === quizData.questions.length - 1;
    nextButton.style.display = 'inline-block';
    submitButton.style.display = 'none';

    // Change next button to show results on last question
    if (currentQuestionIndex === quizData.questions.length - 1) {
        nextButton.style.display = 'none';
        const backButton = document.createElement('button');
        backButton.className = 'nav-button';
        backButton.textContent = '← Back to Results';
        backButton.onclick = showResults;
        document.querySelector('.navigation-buttons').appendChild(backButton);
    } else {
        const backButton = document.querySelector('.navigation-buttons button:last-child');
        if (backButton && backButton.textContent.includes('Back to Results')) {
            backButton.remove();
        }
    }
}

// Restart quiz
function restart() {
    resultsSection.classList.remove('active');
    uploadSection.classList.add('active');
    quizData = null;
    currentQuestionIndex = 0;
    userAnswers = [];
    questionMappings = [];
    score = 0;
    fileInput.value = '';
}

// Cat Break Functions
function openCatBreak() {
    catModal.classList.add('active');
    loadRandomCat();
    startCatRotation();
}

function closeCatBreak() {
    catModal.classList.remove('active');
    stopCatRotation();
}

function getCatApiKey() {
    const apiKey = "live_ml6laxDhhPXDlJJreZHLutyfL5YnuL6dq6LpFR1T1trw5LsXNaQRiOR2xsUszZpn";
    return apiKey || 'DEMO-API-KEY';
}

async function loadRandomCat() {
    try {
        const apiKey = getCatApiKey();
        const headers = new Headers({
            "Content-Type": "application/json",
            "x-api-key": apiKey
        });

        const requestOptions = {
            method: 'GET',
            headers: headers,
            redirect: 'follow'
        };

        const response = await fetch(
            "https://api.thecatapi.com/v1/images/search?size=med&mime_types=jpg&format=json&has_breeds=true&order=RANDOM&page=0&limit=1",
            requestOptions
        );

        const result = await response.json();

        if (result && result.length > 0) {
            const catData = result[0];

            // Update image
            catImage.classList.remove('loaded');
            catImage.src = catData.url;

            // Update breed info
            if (catData.breeds && catData.breeds.length > 0) {
                const breed = catData.breeds[0];
                catBreed.textContent = `Breed: ${breed.name}`;
            } else {
                catBreed.textContent = 'Breed: Mystery Cat 🐱';
            }

            // Show image when loaded
            catImage.onload = () => {
                catImage.classList.add('loaded');
            };
        }
    } catch (error) {
        console.error('Error loading cat:', error);
        catBreed.textContent = 'Failed to load cat 😿';
    }
}

function startCatRotation() {
    let countdown = 5;
    catCountdown.textContent = countdown;

    // Update countdown every second
    catCountdownInterval = setInterval(() => {
        countdown--;
        if (countdown <= 0) {
            countdown = 5;
            loadRandomCat();
        }
        catCountdown.textContent = countdown;
    }, 1000);

    // Rotate cat every 5 seconds
    catBreakInterval = setInterval(() => {
        loadRandomCat();
    }, 5000);
}

function stopCatRotation() {
    if (catBreakInterval) {
        clearInterval(catBreakInterval);
        catBreakInterval = null;
    }
    if (catCountdownInterval) {
        clearInterval(catCountdownInterval);
        catCountdownInterval = null;
    }
}
