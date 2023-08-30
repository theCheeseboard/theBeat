import { Router } from 'itty-router'
import { Env } from './worker';

interface LastFmGetQuery {
	method: "auth.getToken" | "auth.getSession";
}

export const LastFmRouter = Router({
	base: "/lastfm"
});

LastFmRouter.get("/", async (request, env: Env) => {
	const query = request.query as unknown as LastFmGetQuery;

	switch (query.method) {
		case 'auth.getToken':
		case 'auth.getSession':
			break;
		default:
			return new Response("Bad Method", {
				status: 400
			});
	}

	const response = await callLastFm({
		...query
	}, env);

	const body = await response.arrayBuffer();

	return new Response(body, {
		status: response.status,
		headers: {
			"X-theBeat-Api-Key": env.LASTFM_API_KEY,
			"Content-Type": "application/json"
		}
	});
})

async function callLastFm(args: any, env: Env) {
	const modifiedArgs = {
		...args,
		"api_key": env.LASTFM_API_KEY
	};
	const params = Object.keys(modifiedArgs).sort().map(key => `${key}${modifiedArgs[key]}`).join("") + env.LASTFM_SHARED_SECRET;

	const encoder = new TextEncoder();
	const data = encoder.encode(params);
	const hash = await crypto.subtle.digest("MD5", data);
	const hex =  Array.prototype.map.call(new Uint8Array(hash), x => ('00' + x.toString(16)).slice(-2)).join('');

	const argsWithSig = {
		...modifiedArgs,
		"format": "json",
		"api_sig": hex
	};

	console.log(params);

	const url = `https://ws.audioscrobbler.com/2.0/?${new URLSearchParams(argsWithSig).toString()}`;
	console.log(url);

	return await fetch(url, {
		headers: {
			"User-Agent": "theBeatProxy/1.0 (vicr12345@gmail.com)"
		}
	})
}
