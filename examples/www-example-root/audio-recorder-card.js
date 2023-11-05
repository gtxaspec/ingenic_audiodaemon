// Dynamically load recorder.js from a URL, adjust accordingly
const recorderScript = document.createElement('script');
recorderScript.src = '/local/library/recorder.js';
document.head.appendChild(recorderScript);

const SERVER_IP = "192.168.2.1";

recorderScript.onload = () => {
    class AudioRecorderCard extends HTMLElement {
        constructor() {
            super();
            this.attachShadow({ mode: 'open' });
        }

        set hass(hass) {
            if (!this.content) {
                const card = document.createElement('ha-card');
                card.header = 'Two-Way Audio';

                const style = document.createElement('style');
                style.textContent = `
                    ha-card {
                        --ha-card-header-font-size: 20px;
                    }

                    #recordButton {
                        font-size: 22px;
                        padding: 15px 15px;
                        margin-bottom: 10px;
                        cursor: pointer;
                    }

                    #sampleRateSelector {
                        font-size: 22px;
                        padding: 5px 5px;
                        margin-bottom: 10px;
                        margin-right: 20px;
                        cursor: pointer;
                    }
                `;
                card.appendChild(style);

                this.content = document.createElement('div');
                this.content.className = 'card-content';

                const sampleRateSelector = document.createElement('select');
                sampleRateSelector.id = 'sampleRateSelector';
                sampleRateSelector.innerHTML = `
                    <option value="8000">8,000 Hz</option>
                    <option value="16000" selected>16,000 Hz</option>
                    <option value="24000">24,000 Hz</option>
                    <option value="32000">32,000 Hz</option>
                    <option value="44100">44,100 Hz</option>
                    <option value="48000">48,000 Hz</option>
                    <option value="96000">96,000 Hz</option>
                `;
                this.content.appendChild(sampleRateSelector);

                const button = document.createElement('button');
                button.id = 'recordButton';
                button.innerText = 'Push and Hold to Talk';
                this.content.appendChild(button);

                card.appendChild(this.content);
                this.shadowRoot.appendChild(card);

                const recordButton = this.content.querySelector("#recordButton");

                recordButton.addEventListener("pointerdown", this.startRecording.bind(this));
                recordButton.addEventListener("pointerup", () => setTimeout(this.stopRecording.bind(this), 300));
                recordButton.addEventListener("touchend", () => setTimeout(this.stopRecording.bind(this), 300));
                recordButton.addEventListener("touchmove", this.stopRecording.bind(this));  // Handling unintentional drags
            }
        }

        startWebSocket() {
            // WebSocket initialization
            // Use the WebSocket IP address from the configuration or the default
            this.ws = new WebSocket(`wss://${this.websocket_address}${this.websocket_endpoint}`, "audio-protocol");
            this.ws.onopen = function() {
                console.log("WebSocket connection opened");
            };
            this.ws.onerror = function(error) {
                console.error("WebSocket Error:", error);
            };
            this.ws.onclose = function() {
                console.log("WebSocket connection closed");
            };
        }

        sendAudioChunk(chunk) {
            if (this.ws && this.ws.readyState === WebSocket.OPEN) {
                this.ws.send(chunk.buffer);
            }
        }

        async startRecording(event) {
            event.preventDefault();

            // Open WebSocket connection
            this.startWebSocket();

            // Get selected sample rate
            let selectedSampleRate = parseInt(this.content.querySelector("#sampleRateSelector").value);

            this.audioContext = new (window.AudioContext || window.webkitAudioContext)({ sampleRate: selectedSampleRate });
            this.stream = await navigator.mediaDevices.getUserMedia({ audio: true });
            let source = this.audioContext.createMediaStreamSource(this.stream);

            // Always create a new Recorder instance
            if (this.recorder) {
                this.recorder.clear();
            }
            this.recorder = new Recorder(source, { numChannels: 1, targetSampleRate: selectedSampleRate });
            this.recorder.record();
            this.content.querySelector("#recordButton").innerText = "Listening...";

            this.recorder.node.onaudioprocess = (e) => {
                if (!this.recorder.recording) return;

                // Get the audio buffer data
                let input = e.inputBuffer.getChannelData(0);
                let resampledInput = this.resample(input, this.audioContext.sampleRate, this.recorder.config.targetSampleRate);

                let chunk = new Int16Array(resampledInput.length);
                for (let i = 0; i < resampledInput.length; i++) {
                    chunk[i] = Math.min(1, resampledInput[i]) * 0x7FFF;
                }
                this.sendAudioChunk(chunk);
            };
        }

        resample(data, sourceSampleRate, targetSampleRate) {
            if (sourceSampleRate === targetSampleRate) {
                return data;
            }

            var ratio = sourceSampleRate / targetSampleRate;
            var newData = new Float32Array(Math.round(data.length / ratio));

            var offsetResult = 0;
            var offsetSource = 0;

            while (offsetSource < data.length) {
                newData[offsetResult] = data[Math.round(offsetSource)];
                offsetResult++;
                offsetSource += ratio;
            }

            return newData;
        }

        stopRecording(event = null) {
            if (event) {
                event.preventDefault();
            }

            if (this.recorder) {
                this.recorder.stop();
                this.stream.getTracks().forEach(track => track.stop());
            }

            this.content.querySelector("#recordButton").innerText = "Push and Hold to Talk";

            // Close the WebSocket after sending data
            if (this.ws) {
                this.ws.close();
                this.ws = null;
            }

            if (this.audioContext) {
                this.audioContext.close().then(() => {
                    this.audioContext = null;
                });
            }
        }

        setConfig(config) {
            this._config = config;

            // Set the WebSocket IP address from the configuration
            if (config.websocket_address) {
                this.websocket_address = config.websocket_address;
            } else {
                // Default to the hardcoded IP if no config is provided
                this.websocket_address = SERVER_IP;
            }

            // Set the WebSocket endpoint from the configuration
            if (config.websocket_endpoint) {
                this.websocket_endpoint = config.websocket_endpoint;
            } else {
                // Default endpoint if not specified
                this.websocket_endpoint = "/";
            }
        }
    }

    customElements.define('audio-recorder-card', AudioRecorderCard);
};
