function sendPostRequest(formData) {
	return fetch('/', {
		method: 'POST',
		body: formData,
	})
	.then(response => {
		if (response.ok) {
			console.log('POST request successful');
		} else {
			console.error('Failed to send POST request');
			throw new Error('Failed to send POST request');
		}
	})
	.catch(error => {
		console.error('Error sending POST request:', error);
		throw error;
	});
}

document.getElementById('uploadForm').addEventListener('submit', function (event) {
	event.preventDefault(); // Prevent default form submission

	const formData = new FormData(this); // Create FormData object from the form

	// Your logic to send the POST request here
	sendPostRequest(formData)
		.then(() => {
			// Refresh the page after the POST request is successful
			location.reload();
		})
		.catch(error => console.error('Error sending POST request:', error));
});

// Function to send GET request for a file
const sendGetRequest = (fileName) => {
	fetch(`/cgi-bin/uploads/${fileName}`, {
		method: 'GET',
	})
	.then(response => response.blob())
	.then(blob => {
		// Create a link element and trigger the download
		const link = document.createElement('a');
		link.href = URL.createObjectURL(blob);
		link.download = fileName;
		document.body.appendChild(link);
		link.click();
		document.body.removeChild(link);
	})
	.catch(error => console.error('Error downloading file:', error));
};

// Function to send DELETE request for a file
const sendDeleteRequest = (fileName) => {

	return new Promise((resolve, reject) => {
		fetch(`/cgi-bin/uploads/${fileName}`, {
			method: 'DELETE',
		})
		.then(response => {
			if (response.ok) {
				console.log(`File ${fileName} deleted successfully`);
				resolve();
			} else {
				console.error(`Failed to delete file ${fileName}`);
				reject();
			}
		})
		.catch(error => {
			console.error('Error deleting file:', error);
			reject();
		});
	});
};

// Fetch folder contents using JavaScript
fetch('/folder-contents.php')
	.then(response => response.json())
	.then(data => {
		const fileList = document.getElementById('fileList');

		data.forEach(item => {
			const listItem = document.createElement('li');
			listItem.textContent = item;

			const getButton = document.createElement('button');
			getButton.textContent = 'GET';
			getButton.addEventListener('click', () => sendGetRequest(item));

			const deleteButton = document.createElement('button');
			deleteButton.textContent = 'DELETE';
			deleteButton.addEventListener('click', () => {
				sendDeleteRequest(item)
				.then(() => location.reload())
				.catch(error => console.error('Error deleting file:', error));
			   });
			listItem.appendChild(getButton);
			listItem.appendChild(deleteButton);

			fileList.appendChild(listItem);
		});
	})
	.catch(error => console.error('Error fetching folder contents:', error));