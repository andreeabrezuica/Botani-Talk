window.onload = () => {
    updateCircles();
    getCurrentStatus();
    setInterval(getCurrentStatus, 1000);
    fetchDataAndUpdateChart();
    setInterval(fetchDataAndUpdateChart, 60000);
  }

  function getCurrentStatus() {
    let request = new XMLHttpRequest();
    request.open('GET', '/status');
    request.send();

    request.onload = () => {
      if (request.status == 200) {
        const response = JSON.parse(request.response);
        console.log(response);
        document.getElementById('moisture_level').setAttribute('data-num', response.moist);
        document.getElementById('light_level').setAttribute('data-num', response.light);
        if (!isNaN(parseFloat(response.temp)))
          document.querySelector('#temperature p:first-child').innerHTML = response.temp + " &deg;C";
        updateCircles();
        document.getElementById('lights').innerHTML = response.lights_on ? 'ON' : 'OFF';
        document.getElementById('pump').innerHTML = response.pump_cooldown ? 'Cooldown' : (response.pump_on ? 'ON' : 'OFF');
      } else {
        console.err(`Error ${request.status}: ${request.statusText}`);
      }
    }
  }

  function updateCircles() {
    const progressBars = document.querySelectorAll('.progress-item');
    progressBars.forEach((number) => {
      let interval = setInterval(() => {
        const dataValue = parseInt(number.getAttribute('data-value') || '0');
        const dataNum = parseInt(number.getAttribute('data-num'));
        const dataColor = number.getAttribute('data-color');

        if (isNaN(dataValue) || isNaN(dataNum)) {
          clearInterval(interval);
          return;
        }

        if (dataValue === dataNum) {
          clearInterval(interval);
          return;
        }

        const increment = (dataValue < dataNum) ? 1 : -1;
        const newValue = dataValue + increment;

        number.setAttribute('data-value', `${newValue}%`);
        number.style.background = `conic-gradient(${dataColor} calc(${newValue}%), lightgray 0deg)`;
      }, 10);
    });
  }

  const lineChart = new Chart(document.getElementById('lineChart').getContext('2d'), {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: 'Moisture',
        data: [],
        borderColor: 'rgb(54, 162, 235)',
        borderWidth: 2,
        fill: false,
        tension: .4
      }, {
        label: 'Light',
        data: [],
        borderColor: 'rgb(255, 205, 86)',
        borderWidth: 2,
        fill: false,
        tension: .4
      }, {
        label: 'Temperature',
        data: [],
        borderColor: 'rgb(255, 99, 132)',
        borderWidth: 2,
        fill: false,
        tension: .4
      }]
    },
    options: {
      scales: {
        x: {
          type: 'category'
        }
      },
      responsive: true
    }
  });

  function fetchDataAndUpdateChart() {
    fetch('/sensors')
      .then(response => response.text())
      .then(csv => {
        const rows = csv.trim().split('\n').slice(1); // Skip header
        const data = rows.map(row => {
          const [moisture, light, temperature, time] = row.split(',');
          // Check for valid data
          if (!isNaN(parseInt(moisture)) && !isNaN(parseInt(light)) && !isNaN(parseFloat(temperature)) && !isNaN(parseInt(time))) {
            return {
              moisture: parseInt(moisture),
              light: parseInt(light),
              temperature: parseFloat(temperature),
              time: new Date(parseInt(time) * 1000) // Convert UNIX time to milliseconds
            };
          } else {
            return null; // Invalid data
          }
        }).filter(entry => entry !== null); // Remove null entries

        // Prepare data for Chart.js
        const labels = data.map(entry => entry.time.toLocaleString()); // Convert time to readable format
        const moistureData = data.map(entry => entry.moisture);
        const lightData = data.map(entry => entry.light);
        const temperatureData = data.map(entry => entry.temperature);

        // Update chart with new data
        lineChart.data.labels = labels;
        lineChart.data.datasets[0].data = moistureData;
        lineChart.data.datasets[1].data = lightData;
        lineChart.data.datasets[2].data = temperatureData;
        lineChart.update();
        console.log(response);
      })
      .catch(error => {
        console.error('Error fetching data:', error);
      });
  }
