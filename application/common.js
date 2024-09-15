function durationToString(ms, zeroIsInfinity) {
    if (zeroIsInfinity && ms === 0) return "∞";

    let parts = [];

    const seconds = Math.floor(ms / 1000) % 60;
    const minutes = Math.floor(ms / 1000 / 60) % 60;
    const hours = Math.floor(ms / 1000 / 60 / 60);

    if (hours > 0) parts.push(`${hours}`);
    parts.push(`${minutes}`.padStart(2, '0'));
    parts.push(`${seconds}`.padStart(2, '0'));

    return parts.join(":");
}
