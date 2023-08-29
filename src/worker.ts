/**
 * Welcome to Cloudflare Workers! This is your first worker.
 *
 * - Run `npm run dev` in your terminal to start a development server
 * - Open a browser tab at http://localhost:8787/ to see your worker in action
 * - Run `npm run deploy` to publish your worker
 *
 * Learn more at https://developers.cloudflare.com/workers/
 */

import { Router } from 'itty-router'
import { LastFmRouter } from './lastfm';

// Create a new router
const router = Router()

router.all("/lastfm/*", LastFmRouter.handle);

/*
Our index route, a simple hello world.
*/
router.get("/", () => {
	return new Response("Hello, world! This is the root page of your Worker template.")
})

router.all("*", () => {
	return new Response("4.")
})

export interface Env {
	LASTFM_API_KEY: string
	LASTFM_SHARED_SECRET: string
}

export default {
	async fetch(request: Request, env: Env, ctx: ExecutionContext): Promise<Response> {
		return await router.handle(request, env);
	},
};
