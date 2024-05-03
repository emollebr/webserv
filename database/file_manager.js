document.addEventListener('DOMContentLoaded', function () {
    // Form submission handler
    document.getElementById('uploadForm').addEventListener('submit', function (event) {
        event.preventDefault(); // Prevent default form submission
        var formData = new FormData(this); // Create FormData object
        // Send POST request to server with FormData
        fetch('/', {
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
                alert('Failed to upload file!');
            }
        })
        .catch(error => {
            console.error('Error:', error);
            alert('An error occurred while uploading file.');
        });
    });

function refreshFileList() {
    fetch('/list_files') // Send GET request to server to get list of files
    .then(response => response.text()) // Parse response as text
    .then(files => {
        var fileList = document.getElementById('fileList');
        fileList.innerHTML = ''; // Clear existing file list

        // Split the response into an array of file names
        var fileLines = files.split('\n');

        // Add each file to the file list
        fileLines.forEach(file => {
            var trimmedFile = file.trim();
            if (trimmedFile !== '') {
                var listItem = document.createElement('div');
                listItem.innerHTML = `
                    <span>${trimmedFile}</span>
                    <button class="deleteButton" data-filename="${trimmedFile}">Delete</button>
                    <button class="downloadButton" data-filename="${trimmedFile}">Download</button>
                    <button class="viewButton" data-filename="${trimmedFile}">View</button> <!-- Add View button -->
                `;
                fileList.appendChild(listItem);
            }
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
            fetch(`database/uploads/${filename}`, {
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
            fetch(`database/uploads/${filename}`, {
                method: 'GET'
            })
            .then(response => {
                if (response.ok) {
                    // Trigger the file download
                    response.blob().then(blob => {
                        const url = window.URL.createObjectURL(blob);
                        const a = document.createElement('a');
                        a.href = url;
                        a.download = filename;
                        document.body.appendChild(a);
                        a.click();
                        window.URL.revokeObjectURL(url);
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
            window.open(`uploads/${filename}`, '_blank'); // Open file in new tab
        }
    });
});
