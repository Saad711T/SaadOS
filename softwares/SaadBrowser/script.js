document.getElementById("search-button").addEventListener("click", function () {
  const searchInput = document.getElementById("search-input").value.trim();
  if (searchInput) {
    const url = "https://www.google.com/search?q=" + encodeURIComponent(searchInput);
    window.location.href = url; 
  }
});

document.querySelectorAll(".tab-button").forEach(button => {
  button.addEventListener("click", () => {
    const url = button.getAttribute("data-url");
    window.location.href = url;
  });
});
