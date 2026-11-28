type Reading = {
  name: string;
  raw: number;
  percentage: number;
};

async function fetchReadings(): Promise<Reading[]> {
  if (!process.env.ESP32_URL) {
    throw new Error("ESP32_URL environment variable is not set");
  }

  const response = await fetch(`${process.env.ESP32_URL}/api/readings`, {
    signal: AbortSignal.timeout(5000),
  });

  // Always handle HTTP errors (fetch only throws on network failure)
  if (!response.ok) {
    throw new Error(`HTTP error! Status: ${response.status}`);
  }

  return await response.json();
}

try {
  const readings = await fetchReadings();
  console.log(readings);
} catch (error) {
  // `error` is `unknown` here: anything can be thrown in JS, not just Errors.
  // `instanceof Error` narrows it so TS knows `.name` exists.
  process.exitCode = 1;

  if (error instanceof Error && error.name === "TimeoutError") {
    console.error("ESP32 unreachable", { error });
  } else {
    console.error(error);
  }
}
