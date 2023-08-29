import { Router } from 'itty-router'
import { Env } from './worker';

interface LastFmQuery {
	method: "auth.getToken";
}

export const LastFmRouter = Router({
	base: "/lastfm"
});

LastFmRouter.get("/", async (request, env: Env) => {
	const query = request.query as unknown as LastFmQuery;

	switch (query.method) {
		case 'auth.getToken':
			break;
		default:
			return new Response("Bad Method", {
				status: 400
			});
	}

	const response = await callLastFm({
		...query
	}, env);
	return response;
})

async function callLastFm(args: any, env: Env) {
	const modifiedArgs = {
		...args,
		"format": "json",
		"api_key": env.LASTFM_API_KEY
	};
	const params = Object.keys(modifiedArgs).map(key => `${key}${modifiedArgs[key]}`).join("") + env.LASTFM_SHARED_SECRET;

	const encoder = new TextEncoder();
	const data = encoder.encode(params);
	const hash = await crypto.subtle.digest("MD5", data);
	const hex =  Array.prototype.map.call(new Uint8Array(hash), x => ('00' + x.toString(16)).slice(-2)).join('');

	const argsWithSig = {
		...modifiedArgs,
		"api_sig": hex
	};

	return await fetch(`https://ws.audioscrobbler.com/2.0/?${new URLSearchParams(argsWithSig).toString()}`)
}
