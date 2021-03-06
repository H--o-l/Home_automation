#!/usr/bin/env python3

from aiohttp import web
import logging


routes = web.RouteTableDef()


@routes.get('/')
async def home(req):
    text = "Hello world\n"
    return web.Response(text=text)

app = web.Application()
app.router.add_routes(routes)
logging.basicConfig(level=logging.DEBUG)

if __name__ == '__main__':
    web.run_app(app, port=6000)
