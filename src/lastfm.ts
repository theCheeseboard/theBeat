import { Router } from 'itty-router'
import { Env } from './worker';

interface LastFmPostQuery {
	method: "track.scrobble" | "track.updateNowPlaying";
	[key: string]: any
}

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

LastFmRouter.post("/", async (request, env: Env) => {
	const json = await request.json() as LastFmPostQuery;

	switch (json.method) {
		case 'track.scrobble':
		case 'track.updateNowPlaying':
			break;
		default:
			return new Response("Bad Method", {
				status: 400
			});
	}

	const response = await postLastFm(json, env);

	const body = await response.arrayBuffer();
	return new Response(body, {
		status: response.status,
		headers: {
			"X-theBeat-Api-Key": env.LASTFM_API_KEY,
			"Content-Type": "application/json"
		}
	});
})

async function transformArgs(args: any, env: Env) {
	const modifiedArgs = {
		...args,
		"api_key": env.LASTFM_API_KEY
	};
	const params = Object.keys(modifiedArgs).sort().map(key => `${key}${modifiedArgs[key]}`).join("") + env.LASTFM_SHARED_SECRET;

	const encoder = new TextEncoder();
	const data = encoder.encode(params);
	const hash = await crypto.subtle.digest("MD5", data);
	const hex =  Array.prototype.map.call(new Uint8Array(hash), x => ('00' + x.toString(16)).slice(-2)).join('');

	return {
		...modifiedArgs,
		"format": "json",
		"api_sig": hex
	};
}

async function callLastFm(args: any, env: Env) {
	const transformedArgs = await transformArgs(args, env);
	const url = `https://ws.audioscrobbler.com/2.0/?${new URLSearchParams(transformedArgs).toString()}`;
	return await fetch(url, {
		headers: {
			"User-Agent": "theBeatProxy/1.0 (vicr12345@gmail.com)"
		}
	})
}

async function postLastFm(args: any, env: Env) {
	const transformedArgs = await transformArgs(args, env);
	const url = "https://ws.audioscrobbler.com/2.0/";
	return await fetch(url, {
		headers: {
			"User-Agent": "theBeatProxy/1.0 (vicr12345@gmail.com)"
		},
		method: "POST",
		body: new URLSearchParams(transformedArgs).toString()
	});
}
