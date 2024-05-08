document.addEventListener('DOMContentLoaded', function () {
    // Form submission handler
    document.getElementById('uploadForm').addEventListener('submit', function (event) {
        event.preventDefault(); // Prevent default form submission
        var formData = new FormData(this); // Create FormData PATH
        // Send POST request to server with FormData
        fetch('/uploads', {
            method: 'POST',
            body: formData
        })
        .then(response => {
            if (response.ok) {
                alert('File uploaded successfully!');
                document.getElementById('uploadForm').reset();
                // Refresh file list after successful upload
                refreshFileList();
            } else {
                // Handle non-successful response
                response.text().then(errorMessage => {
                    console.error('Error:', errorMessage);
                    alert('Failed to upload file: ' + errorMessage);
                });
            }
        })
        .catch(error => {
            console.error('Error:', error);
            alert('An error occurred while uploading file.');
        });
        
    });

    function refreshFileList() {
        fetch('/uploads') // Send GET request to server to get list of files
        .then(response => response.text()) // Parse response as text
        .then(html => {
            var fileListContainer = document.getElementById('fileList');
            fileListContainer.innerHTML = ''; // Clear existing content
    
            // Create a temporary div to parse the HTML
            var tempDiv = document.createElement('div');
            tempDiv.innerHTML = html;
    
            // Get all the hyperlinks from the temporary div
            var links = tempDiv.querySelectorAll('a');
    
            // Iterate over each hyperlink
            links.forEach(link => {
                var listItem = document.createElement('div');
    
                // Create delete button
                var deleteButton = document.createElement('button');
                deleteButton.classList.add('deleteButton');
                deleteButton.textContent = 'Delete';
                deleteButton.setAttribute('data-filename', link.getAttribute('href'));
    
                // Create download button
                var downloadButton = document.createElement('button');
                downloadButton.classList.add('downloadButton');
                downloadButton.textContent = 'Download';
                downloadButton.setAttribute('data-filename', link.getAttribute('href'));
    
                // Append hyperlink, download button, and delete button to list item
                listItem.appendChild(link);
                listItem.appendChild(downloadButton);
                listItem.appendChild(deleteButton);
    
                // Append list item to file list container
                fileListContainer.appendChild(listItem);
            });
    
            // Add event listeners for delete buttons in the file list
            var deleteButtons = document.querySelectorAll('.deleteButton');
            deleteButtons.forEach(button => {
                button.addEventListener('click', function (event) {
                    var filename = event.target.getAttribute('data-filename');
                    // Send DELETE request to server with filename
                    fetch(`${filename}`, {
                        method: 'DELETE'
                    })
                    .then(response => {
                        if (response.ok) {
                            alert('File deleted successfully!');
                            // Refresh file list after successful delete
                            refreshFileList();
                        } else {
                            alert('Failed to delete file!');
                        }
                    })
                    .catch(error => {
                        console.error('Error:', error);
                        alert('An error occurred while deleting file.');
                    });
                });
            });
    
            // Add event listeners for download buttons in the file list
            var downloadButtons = document.querySelectorAll('.downloadButton');
            downloadButtons.forEach(button => {
                button.addEventListener('click', function (event) {
                    var filename = event.target.getAttribute('data-filename');
                    // Open the file in a new tab for download
                    window.open(`${filename}`, '_blank');
                });
            });
        })
        .catch(error => {
            console.error('Error:', error);
            alert('An error occurred while fetching file list.');
        });
    }
    

    // Initial file list refresh
    refreshFileList();

    // Event delegation for delete buttons
    document.addEventListener('click', function (event) {
        if (event.target.classList.contains('deleteButton')) {
            var filename = event.target.getAttribute('data-filename');
            // Send DELETE request to server with filename
            fetch(`${filename}`, {
                method: 'DELETE'
            })
            .then(response => {
                if (response.ok) {
                    alert('File deleted successfully!');
                    // Refresh file list after successful delete
                    refreshFileList();
                } else {
                    alert('Failed to delete file!');
                }
            })
            .catch(error => {
                console.error('Error:', error);
                alert('An error occurred while deleting file.');
            });
        }
    });

    document.addEventListener('click', function (event) {
        if (event.target.classList.contains('downloadButton')) {
            var filename = event.target.getAttribute('data-filename');
            // Send GET request to server to download the file
            fetch(`${filename}`, {
                method: 'GET'
            })
            .then(response => {
                if (response.ok) {
                    // Trigger the file download
                    response.blob().then(blob => {
                        const url = window.URL.createPATHURL(blob);
                        const a = document.createElement('a');
                        a.href = url;
                        a.download = filename;
                        document.body.appendChild(a);
                        a.click();
                        window.URL.revokePATHURL(url);
                    });
                } else {
                    alert('Failed to download file!');
                }
            })
            .catch(error => {
                console.error('Error:', error);
                alert('An error occurred while downloading file.');
            });
        } else if (event.target.classList.contains('viewButton')) { // View button click handler
            var filename = event.target.getAttribute('data-filename');
            window.open(`${filename}`, '_blank'); // Open file in new tab
        }
    });
});
