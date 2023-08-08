create database Operativos;
use Operativos;
drop database Operativos;

create table cliente(
	id_cliente int not null auto_increment,
    usuario varchar(10) not null,
    contraseña varchar(10) not null,
    primary key (id_cliente)
);

create table producto(
	id_producto int not null auto_increment,
    nombre varchar(30) not null,
    marca varchar(15) not null, 
    cantidad int not null,
    precio decimal(6,2) not null,
    primary key (id_producto)
);

create table carrito(
	id_carrito int not null auto_increment,
    id_cliente int not null,
    id_producto int not null,
    cantidad int,
    precio decimal(6,2),
    total decimal(6,2),
    primary key (id_carrito),
    foreign key(id_cliente) references cliente(id_cliente),
    foreign key(id_producto) references producto(id_producto)
);

show create table cliente;
select * from carrito;
select contraseña from cliente where contraseña = 'Alvares' and usuario = ;
SELECT id_cliente FROM cliente WHERE contraseña = 'Said153';
SELECT id_cliente FROM cliente WHERE usuario = 'Omar';

insert into cliente(id_cliente, usuario, contraseña)
values ('3', 'Joshua', 'jalves');
values ('2', 'Dario', 'Alvares');
values ('1', 'Omar','Said153');
SELECT id_cliente FROM cliente WHERE usuario = 'Dario'

insert into producto(id_producto, nombre, marca, cantidad, precio)
values ('2', 'Jordan','Nike', '15', '5000.00');
update carrito set cantidad = cantidad + '4' where id_carrito = '1'

select * from carrito
select * from producto
select * from cliente

insert into carrito( id_carrito ,id_cliente, id_producto, cantidad, precio,total)
values ('2','2' ,'2', '2', '6500.00', '5600.00');

select id_producto, cantidad, precio, total from carrito inner join cliente on carrito.id_cliente = cliente.id_cliente