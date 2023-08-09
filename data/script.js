// Get references to HTML elements
const headerElement = document.getElementById("header");
const paragraphElement = document.getElementById("paragraph");
const updateButton = document.getElementById("updateButton");

// Add an event listener to the button
updateButton.addEventListener("click", () => {
    // Update the content of the header and paragraph
    console.log("button clicked!");
    headerElement.textContent = "New Header";
    paragraphElement.textContent = "New Paragraph Content";
});